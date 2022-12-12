#ifdef CONFIG_ENABLE_MODBUS

#include "ModbusProtocol.h"
#include <unistd.h>
#include "Log.h"

static void ReadDeviceDataProcess(ModbusProtocol *modbusProtocol);

ModbusProtocol *modbusProtocol = NULL;

ModbusProtocol::ModbusProtocol(string name, string type, string device, uint32_t baudrate, string config)
{
	this->name = name;
	this->type = type;
	this->device = device;
	this->baudrate = baudrate;
	this->config = config;
	ctx = NULL;
	if (init() == -1)
	{
		LOGE("Init Modbus error");
		exit(1);
	}
	Start();
}

ModbusProtocol::~ModbusProtocol()
{
	Stop();
}

int ModbusProtocol::Start()
{
	modbusThread = new thread(ReadDeviceDataProcess, this);
	modbusThread->detach();
	return 0;
}

int ModbusProtocol::Stop()
{
	// TODO: Stop modbusThread
	if (ctx)
	{
		modbus_close(ctx);
		modbus_free(ctx);
		ctx = NULL;
	}
	return 0;
}

int ModbusProtocol::init()
{
	if (ctx)
		return -1;
	if (type == "TCP")
	{
		ctx = modbus_new_tcp(device.c_str(), baudrate);
	}
	else if (type == "RTU")
	{
		if (config.size() == 3)
		{
			char parity = config.c_str()[0];
			int dataBits = config.c_str()[1] - 48;
			int stopBits = config.c_str()[2] - 48;
			ctx = modbus_new_rtu(device.c_str(), baudrate, parity, dataBits, stopBits);
		}
		else
			ctx = modbus_new_rtu(device.c_str(), baudrate, 'N', 8, 1);
	}

	if (!ctx)
	{
		LOGE("New modbus error");
		return -1;
	}

	if (modbus_connect(ctx) == -1)
	{
		LOGE("Modbus connection failed: %s", modbus_strerror(errno));
		modbus_free(ctx);
		ctx = NULL;
		return -1;
	}
	return 0;
}

void ModbusProtocol::AddDeviceModbus(DeviceModbus *deviceModbus)
{
	deviceModbusList.push_back(deviceModbus);
}

void ModbusProtocol::RemoveDeviceModbus(DeviceModbus *deviceModbus)
{
}

static double BuffToData(uint16_t *data, string parameterType, double parameterScale, bool parameterWordSwap)
{
	typedef union
	{
		uint16_t c[1];
		float f;
	} u16f_st;

	typedef union
	{
		uint16_t c[2];
		float f;
	} u32f_st;

	typedef union
	{
		uint16_t c[4];
		float f;
	} u64f_st;

	double double_tmp;
	stringstream ss;
	if (parameterType == "f16")
	{
		u16f_st u16f;
		u16f.c[0] = data[0];
		double_tmp = u16f.f;
	}
	else if (parameterType == "f32")
	{
		u32f_st u32f;
		if (parameterWordSwap)
		{
			u32f.c[0] = data[0];
			u32f.c[1] = data[1];
		}
		else
		{
			u32f.c[0] = data[1];
			u32f.c[1] = data[0];
		}
		double_tmp = u32f.f;
	}
	else if (parameterType == "f64")
	{
		u64f_st u64f;
		if (parameterWordSwap)
		{
			u64f.c[0] = data[0];
			u64f.c[1] = data[1];
			u64f.c[2] = data[2];
			u64f.c[3] = data[3];
		}
		else
		{
			u64f.c[0] = data[3];
			u64f.c[1] = data[2];
			u64f.c[2] = data[1];
			u64f.c[3] = data[0];
		}
		double_tmp = u64f.f;
	}
	else if (parameterType == "u16" || parameterType == "i16")
	{
		double_tmp = data[0];
	}
	else if (parameterType == "i32" || parameterType == "u32")
	{
		if (parameterWordSwap)
			double_tmp = data[0] | data[1] << 16;
		else
			double_tmp = MODBUS_GET_INT32_FROM_INT16(data, 0);
	}
	else
	{
		if (parameterWordSwap)
			double_tmp = data[0] | data[1] << 16 | (uint64_t)data[2] << 32 | (uint64_t)data[3] << 48;
		else
			double_tmp = MODBUS_GET_INT64_FROM_INT16(data, 0);
	}
	return double_tmp * parameterScale;
}

static void ReadDeviceDataProcess(ModbusProtocol *modbusProtocol)
{
	LOGD("ReadDeviceDataProcess");
	int rc;
	time_t currentTime;
	int timeout;
	uint16_t buffer[512];
	int sleepTime;
	while (1)
	{
		sleepTime = 5;
		modbusProtocol->mtx.lock();
		for (auto &deviceModbus : modbusProtocol->deviceModbusList)
		{
			currentTime = time(NULL);
			if (deviceModbus->GetReadTime() > currentTime)
			{
				if (sleepTime > deviceModbus->GetReadTime() - currentTime)
					sleepTime = deviceModbus->GetReadTime() - currentTime;
				continue;
			}
			deviceModbus->SetReadTime(currentTime + deviceModbus->GetScanRate());

			LOGD("Read device %s", deviceModbus->GetName().c_str());

			if (!modbusProtocol->ctx)
			{
				if (modbusProtocol->init() == -1)
				{
					LOGE("Init Modbus error");
					exit(1);
				}
			}

			if (modbusProtocol->type == "RTU")
			{
				rc = modbus_set_slave(modbusProtocol->ctx, deviceModbus->GetAddr());
				if (rc == -1)
				{
					LOGE("modbus_set_slave: %s", modbus_strerror(errno));
					continue;
				}
			}

			timeout = deviceModbus->GetTimeout();
			if (timeout == 0)
				timeout = 1000;
			rc = modbus_set_response_timeout(modbusProtocol->ctx, timeout / 1000, timeout % 1000);
			if (rc == -1)
			{
				LOGE("modbus_set_response_timeout: %s, errno: %d", modbus_strerror(errno), errno);
			}

			deviceModbus->values = Json::Value::null;
			for (auto &modbusParameter : deviceModbus->modbusParameterList)
			{
				modbus_flush(modbusProtocol->ctx);
				uint32_t reg = modbusParameter->GetAddress() / 10000;
				uint32_t parameter_address = (reg == 0 || modbusParameter->GetAddress() < reg * 10000) ? modbusParameter->GetAddress() : modbusParameter->GetAddress() - reg * 10000 - 1;
				// uint32_t parameter_address = modbusParameter->GetAddress();
				rc = -1;
				if (modbusParameter->GetFunction() == READ_MULTI_HOLDING_REGISTER)
					rc = modbus_read_registers(modbusProtocol->ctx, parameter_address, modbusParameter->GetDataLength(), buffer);
				else if (modbusParameter->GetFunction() == READ_INPUT_REGISTER)
					rc = modbus_read_input_registers(modbusProtocol->ctx, parameter_address, modbusParameter->GetDataLength(), buffer);
				else
					continue;
				if (rc == -1)
				{
					LOGW("Read Modbus param \"%s\" error: %s, dev addr: %d, data addr: %d, errno: %d", modbusParameter->GetName().c_str(), modbus_strerror(errno), deviceModbus->GetAddr(), parameter_address, errno);
				}
				else
				{
					LOGV("Read Modbus param \"%s\" OK", modbusParameter->GetName().c_str());
					deviceModbus->values[modbusParameter->GetName()] = BuffToData(buffer, modbusParameter->GetDataType(), modbusParameter->GetScale(), modbusParameter->GetWordSwap());
				}
				usleep(1000);
			}
			deviceModbus->CheckTrigger();
			deviceModbus->PushTelemetry();
			usleep(1000);
		}
		modbusProtocol->mtx.unlock();
		sleep(sleepTime);
	}
}

bool ModbusProtocol::DoDeviceTrigger(DeviceModbus *deviceModbus, Json::Value dataValue)
{
	int rc;
	int timeout;
	mtx.lock();
	if (dataValue.isMember("param") && dataValue["param"].isString() && dataValue.isMember("value") && dataValue["value"].isInt())
	{
		string param = dataValue["param"].asString();
		int value = dataValue["value"].asInt();
		LOGD("param: %s, value: %d", param.c_str(), value);

		if (!ctx)
		{
			if (init() == -1)
			{
				LOGE("Init Modbus error");
				exit(1);
			}
		}

		if (type == "RTU")
		{
			rc = modbus_set_slave(ctx, deviceModbus->GetAddr());
			if (rc == -1)
			{
				LOGE("modbus_set_slave: %s", modbus_strerror(errno));
			}
		}

		timeout = deviceModbus->GetTimeout();
		if (timeout == 0)
			timeout = 1000;
		rc = modbus_set_response_timeout(ctx, timeout / 1000, timeout % 1000);
		if (rc == -1)
		{
			LOGE("modbus_set_response_timeout: %s, errno: %d", modbus_strerror(errno), errno);
		}

		for (auto &modbusParameter : deviceModbus->modbusParameterList)
		{
			if (modbusParameter->GetName() == param)
			{
				LOGV("Write modbus data: %s", modbusParameter->GetName().c_str());
				modbus_flush(ctx);
				uint32_t reg = modbusParameter->GetAddress() / 10000;
				uint32_t parameter_address = (reg == 0 || modbusParameter->GetAddress() < reg * 10000) ? modbusParameter->GetAddress() : modbusParameter->GetAddress() - reg * 10000 - 1;
				// uint32_t parameter_address = modbusParameter->GetAddress();
				rc = -1;
				if (modbusParameter->GetFunction() == WRITE_SINGLE_COIL)
					rc = modbus_write_bit(ctx, parameter_address, value);
				else if (modbusParameter->GetFunction() == WRITE_SINGLE_HOLDING_REGISTER)
					rc = modbus_write_register(ctx, parameter_address, value);
				else if (modbusParameter->GetFunction() == WRITE_MULTI_COIL)
					rc = modbus_write_bits(ctx, parameter_address, modbusParameter->GetDataLength(), (uint8_t *)&value);
				else if (modbusParameter->GetFunction() == WRITE_MULTI_HOLDING_REGISTER)
					rc = modbus_write_registers(ctx, parameter_address, modbusParameter->GetDataLength(), (uint16_t *)&value);
				else
					LOGW("Modbus write function %d not implemented", modbusParameter->GetFunction());
				if (rc == -1)
				{
					LOGW("Write Modbus param \"%s\" error: %s, dev addr: %d, data addr: %d, errno: %d", modbusParameter->GetName().c_str(), modbus_strerror(errno), deviceModbus->GetAddr(), parameter_address, errno);
				}
				else
				{
					LOGV("Write Modbus param \"%s\" OK", modbusParameter->GetName().c_str());
					deviceModbus->values[modbusParameter->GetName()] = value;
					deviceModbus->PushTelemetry();
				}
			}
		}
		mtx.unlock();
	}
	return true;
}

// void ModbusProtocol::DoDeviceTrigger()
// {
// 	for (auto &deviceModbus : deviceModbusList)
// 	{
// 		while (!deviceModbus->doDataList.empty())
// 		{
// 			Json::Value dataValue = deviceModbus->doDataList.back();
// 			deviceModbus->doDataList.pop_back();
// 			DoDeviceTrigger(deviceModbus, dataValue);
// 		}
// 	}
// }

#endif
