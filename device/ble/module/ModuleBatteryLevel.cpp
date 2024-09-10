#include "ModuleBatteryLevel.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Db.h"

ModuleBatteryLevel::ModuleBatteryLevel(Device *device, uint16_t addr) : Module(device, addr)
{
	bat = 0;
}

ModuleBatteryLevel::~ModuleBatteryLevel()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleBatteryLevel::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_BATTERY)
		bat = value;
}

void ModuleBatteryLevel::SaveAttribute()
{
	database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_BATTERY, bat);
}
#endif

int ModuleBatteryLevel::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_BATTERY) && dataValue[KEY_ATTRIBUTE_BATTERY].isInt())
	{
		bat = dataValue[KEY_ATTRIBUTE_BATTERY].asInt();
		// CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleBatteryLevel::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x01 && data[2] == 0x00)
	{
		if (bat != data[4])
		{
			bat = data[4];
#ifdef CONFIG_SAVE_ATTRIBUTE
			SaveAttribute();
#endif
		}
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	else
	{
		typedef struct __attribute__((packed))
		{
			uint16_t opcode;
			uint16_t header;
			uint8_t battery;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)data;
		if (data_message->opcode == G_BATTERY_STATUS)
		{
			if (bat != data_message->battery)
			{
				bat = data_message->battery;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute();
#endif
			}
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

bool ModuleBatteryLevel::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_BATTERY) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_BATTERY].isInt())
		{
			int bat = dataValue[KEY_ATTRIBUTE_BATTERY].asInt();
			rs = Util::CompareNumber(op, this->bat, bat);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_BATTERY].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_BATTERY];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int bat1 = listValue[0].asInt();
				int bat2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->bat, bat1, bat2);
				return true;
			}
		}
	}
	return false;
}

void ModuleBatteryLevel::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_BATTERY] = bat;
}
