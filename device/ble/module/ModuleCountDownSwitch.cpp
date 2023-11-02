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
	id = BLE_ATTRIBUTE_COUNTDOWN;
}

ModuleCountDownSwitch::~ModuleCountDownSwitch()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleCountDownSwitch::InitAttribute(int id, double value)
{
	if (this->id == id)
		time = value;
}

void ModuleCountDownSwitch::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, time);
}
#endif

int ModuleCountDownSwitch::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			time = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}

int ModuleCountDownSwitch::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0xe3 && data[1] == 0x11 && data[2] == 0x02 && data[3] == 0x0b && data[4] == 0x07)
	{
		time = data[6] | (data[7] << 8 );
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleCountDownSwitch::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
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
				rs = Util::CompareNumber(op, this->time, value1, value2);
				return true;
			}
		}
	}
#endif
	return false;
}

void ModuleCountDownSwitch::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_COUNTDOWN] = time;
#else
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = time;
	jsonValue.append(dataValue);
#endif
}

int ModuleCountDownSwitch::Do(Json::Value &dataValue)
{
	LOGV("ModuleTimeActionPir Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (bleProtocol && dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_COUNTDOWN) && dataValue[KEY_ATTRIBUTE_COUNTDOWN].isInt())
	{
		int time = dataValue[KEY_ATTRIBUTE_COUNTDOWN].asInt();
		if (bleProtocol->TimeActionPirLightSensor(addr, time) == CODE_OK)
		{
			this->time = time;
			return CODE_OK;
		}
	}
#else
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			if (bleProtocol)
			{
				bleProtocol->CountDownSwitch(addr, value, 0);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}
