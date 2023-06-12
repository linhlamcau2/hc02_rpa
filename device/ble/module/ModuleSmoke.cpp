#include "ModuleSmoke.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleSmoke::ModuleSmoke(Device *device, uint32_t addr) : Module(device, addr)
{
	smoke = 0;
	power = 0;
	idSmoke = BLE_ATTRIBUTE_SMOKE;
	idPower = BLE_ATTRIBUTE_SMOKE_PIN;
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
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idSmoke == id || this->idPower == id)
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
			{
				if (this->idSmoke == id)
					smoke = dataValue["VALUE"].asInt();
				else if (this->idPower == id)
					power = dataValue["VALUE"].asInt();
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
	}
#endif
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
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleSmoke::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idSmoke == id &&
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

				rs = Util::CompareNumber(op, this->smoke, value1, value2);
				return true;
			}
		}
	}
#endif
	return false;
}

void ModuleSmoke::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_SMOKE] = smoke;
	jsonValue[KEY_ATTRIBUTE_SMOKE_PIN] = power;
#else
	Json::Value dataValue;
	dataValue["ID"] = idSmoke;
	dataValue["VALUE"] = smoke;
	jsonValue.append(dataValue);
	dataValue["ID"] = idPower;
	dataValue["VALUE"] = power;
	jsonValue.append(dataValue);
#endif
}
