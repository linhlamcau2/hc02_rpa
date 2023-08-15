#include "ModuleDoorHangOn.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleDoorHangOn::ModuleDoorHangOn(Device *device, uint16_t addr) : Module(device, addr)
{
	hangOn = 0;
}

ModuleDoorHangOn::~ModuleDoorHangOn()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDoorHangOn::InitAttribute(int id, double value)
{
	if (this->id == id)
		hangOn = value;
}

void ModuleDoorHangOn::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, hangOn);
}
#endif

int ModuleDoorHangOn::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_HANGON) && dataValue[KEY_ATTRIBUTE_HANGON].isInt())
	{
		hangOn = dataValue[KEY_ATTRIBUTE_HANGON].asInt();
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleDoorHangOn::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x09 && data[2] == 0x04)
	{
		hangOn = data[3];
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleDoorHangOn::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_HANGON) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_HANGON].isInt())
		{
			int hangOn = dataValue[KEY_ATTRIBUTE_HANGON].asInt();
			rs = Util::CompareNumber(op, this->hangOn, hangOn);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_HANGON].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_HANGON];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int hangOn1 = listValue[0].asInt();
				int hangOn2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->hangOn, hangOn1, hangOn2);
				return true;
			}
		}
	}
	return false;
}

void ModuleDoorHangOn::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_HANGON] = hangOn;
}
