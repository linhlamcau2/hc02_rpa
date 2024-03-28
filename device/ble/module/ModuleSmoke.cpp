#include "ModuleSmoke.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleSmoke::ModuleSmoke(Device *device, uint16_t addr) : Module(device, addr)
{
	smoke = 0;
	power = 0;
}

ModuleSmoke::~ModuleSmoke()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleSmoke::InitAttribute(int id, double value)
{
	if (this->id == idSmoke)
		smoke = value;
	else if (this->id == idPower)
		power = value;
}

void ModuleSmoke::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idSmoke, smoke);
	database->DeviceAttributeAddOrReplace(device, idPower, power);
}
#endif

int ModuleSmoke::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_SMOKE) && dataValue[KEY_ATTRIBUTE_SMOKE].isInt() &&
			dataValue.isMember(KEY_ATTRIBUTE_SMOKE_PIN) && dataValue[KEY_ATTRIBUTE_SMOKE_PIN].isInt())
	{
		smoke = dataValue[KEY_ATTRIBUTE_SMOKE].asInt();
		power = dataValue[KEY_ATTRIBUTE_SMOKE_PIN].asInt();
		// CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleSmoke::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x08 && data[2] == 0x01)
	{
		typedef struct __attribute__((packed))
		{
			uint8_t smoke;
			uint8_t power;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)&data[3];
		smoke = (data_message->smoke);
		power = (data_message->power);
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
#ifdef __ANDROID__
		if (smoke == 1)
		{
			string id = Util::genRandRQI(16);
			Json::Value tempJson = gateway->BuildJsonDataNoti(device, id, "warning", "cảnh báo có khói");
			Noti *temp = new Noti(id, "warning", tempJson.toString(), to_string(time(NULL)), to_string(time(NULL)));
			gateway->CreateNoti(temp, true, true);
		}
#endif
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleSmoke::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue.isMember(KEY_ATTRIBUTE_SMOKE))
		{
			if (dataValue[KEY_ATTRIBUTE_SMOKE].isInt())
			{
				int smoke = dataValue[KEY_ATTRIBUTE_SMOKE].asInt();
				rs = Util::CompareNumber(op, this->smoke, smoke);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_SMOKE].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_SMOKE];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int smoke1 = listValue[0].asInt();
					int smoke2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->smoke, smoke1, smoke2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_SMOKE_PIN))
		{
			if (dataValue[KEY_ATTRIBUTE_SMOKE_PIN].isInt())
			{
				int power = dataValue[KEY_ATTRIBUTE_SMOKE_PIN].asInt();
				rs = Util::CompareNumber(op, this->power, power);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_SMOKE_PIN].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_SMOKE_PIN];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int power1 = listValue[0].asInt();
					int power2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->power, power1, power2);
					return true;
				}
			}
		}
	}
	return false;
}

void ModuleSmoke::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_SMOKE] = smoke;
	jsonValue[KEY_ATTRIBUTE_SMOKE_PIN] = power;
}
