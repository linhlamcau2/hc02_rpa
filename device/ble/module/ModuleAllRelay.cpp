#include "ModuleAllRelay.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleAllRelay::ModuleAllRelay(Device *device, uint32_t addr) : Module(device, addr)
{
	onoff = 0;
	id = BLE_ATTRIBUTE_CONTROL_ALL_RELAY;
}

ModuleAllRelay::~ModuleAllRelay()
{
}


int ModuleAllRelay::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			onoff = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			// CheckTrigger();
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}

int ModuleAllRelay::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcodeVendor;
		uint16_t vendorId;
		uint16_t header;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcodeVendor == 0xE3 || data_message->header == 0)
	{
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleAllRelay::CheckData(Json::Value &dataValue, bool &rs)
{
	// LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
			dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
			dataValue.isMember("OP") && dataValue["OP"].isString())
		{
			uint16_t value1 = 0, value2 = 0;
			string op = dataValue["OP"].asString();
			Json::Value listValue = dataValue["VALUE"];
			if (listValue.size() > 0)
			{
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					value1 = listValue[0].asInt();
					value2 = listValue[1].asInt();
				}
				else if (listValue.size() == 1 && listValue[0].isInt())
				{
					value1 = listValue[0].asInt();
				}
				rs = Util::CompareNumber(op, this->onoff, value1, value2);
				return true;
			}
		}
	}
#endif
	return false;
}

void ModuleAllRelay::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = onoff;
	jsonValue.append(dataValue);
#endif
}

int ModuleAllRelay::Do(Json::Value &dataValue)
{
	// LOGD("ModuleAllRelay Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
			dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			if (bleProtocol)
			{
				bleProtocol->ControlRelayOfSwitch(addr, device->GetType(), 255, value);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}
