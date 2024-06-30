#include "ModuleCountDownSwitch.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleCountDownSwitch::ModuleCountDownSwitch(Device *device, uint32_t addr) : Module(device, addr)
{
	time = 0;
}

ModuleCountDownSwitch::~ModuleCountDownSwitch()
{
}

int ModuleCountDownSwitch::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_COUNTDOWN) && dataValue[KEY_ATTRIBUTE_COUNTDOWN].isInt())
	{
		time = dataValue[KEY_ATTRIBUTE_COUNTDOWN].asInt();
		// CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleCountDownSwitch::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0xe3 && data[1] == 0x11 && data[2] == 0x02 && data[3] == 0x0b && data[4] == 0x07)
	{
		time = data[6] | (data[7] << 8);
		BuildTelemetryValue(jsonValue);
		CheckTrigger(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleCountDownSwitch::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_DISTANCE) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_DISTANCE].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_DISTANCE].asInt();
			rs = Util::CompareNumber(op, this->time, value);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_DISTANCE].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_DISTANCE];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int value1 = listValue[0].asInt();
				int value2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->time, value1, value2);
				return true;
			}
		}
	}
	return false;
}

void ModuleCountDownSwitch::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_COUNTDOWN] = time;
}

int ModuleCountDownSwitch::Do(Json::Value &dataValue)
{
	LOGV("ModuleTimeActionPir Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_COUNTDOWN) && dataValue[KEY_ATTRIBUTE_COUNTDOWN].isInt())
	{
		int time = dataValue[KEY_ATTRIBUTE_COUNTDOWN].asInt();
		if (bleProtocol->CountDownSwitch(addr, time, 0) == CODE_OK)
		{
			this->time = time;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
