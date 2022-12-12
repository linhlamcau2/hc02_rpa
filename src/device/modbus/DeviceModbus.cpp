#include "DeviceModbus.h"
#include "Log.h"
#include <Util.h>
#include "ModbusProtocol.h"

DeviceModbus::DeviceModbus(string id, string name, string mac, uint32_t addr, uint32_t type) : Device(id, name, mac, addr, type)
{
	serialPort = "";
	baudrate = 9600;
	scanRate = 2;
	timeout = 1;
	protocol = MODBUS_DEVICE;
}

string DeviceModbus::GetSerialPort()
{
	return serialPort;
}

int DeviceModbus::GetBaudrate()
{
	return baudrate;
}

int DeviceModbus::GetScanRate()
{
	return scanRate;
}

int DeviceModbus::GetTimeout()
{
	return timeout;
}

time_t DeviceModbus::GetReadTime()
{
	return readTime;
}

void DeviceModbus::SetReadTime(time_t readTime)
{
	this->readTime = readTime;
}

void DeviceModbus::SetModbusConfig(string serialPort, int baudrate, int scanRate, int timeout)
{
	this->serialPort = serialPort;
	this->baudrate = baudrate;
	this->scanRate = scanRate;
	this->timeout = timeout;
}

void DeviceModbus::AddParameter(ModbusParameter *modbusParameter)
{
	modbusParameterList.push_back(modbusParameter);
}

int DeviceModbus::BuildTelemetryValue(Json::Value &pushDataValue)
{
	if (values.isNull())
	{
		Offline();
		return -1;
	}
	pushDataValue = values;
	return 0;
}

bool DeviceModbus::CheckData(Json::Value &dataValue, bool& rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		for (auto &modbusParameter : modbusParameterList)
		{
			string parameterName = modbusParameter->GetName();
			if (dataValue.isMember(parameterName) && dataValue[parameterName].isInt() && values.isMember(parameterName) && values[parameterName].isInt())
			{
				return Util::CompareNumber(values[parameterName].asInt(), dataValue[parameterName].asInt(), op);
			}
		}
	}
	return false;
}

bool DeviceModbus::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	// doDataList.push_back(dataValue);
	return modbusProtocol->DoDeviceTrigger(this, dataValue);
}
