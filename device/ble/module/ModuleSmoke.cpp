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

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleSmoke::InitAttribute(int id, double value)
{
	if (this->id == idTemp)
		temp = value;
	else if (this->id == idHum)
		hum = value;
}

void ModuleSmoke::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idTemp, temp);
	database->DeviceAttributeAddOrReplace(device, idHum, hum);
}
#endif

bool ModuleSmoke::InputData(uint8_t *data, int len, Json::Value &jsonValue, Json::Value &jsonValueV2)
{
	if (data[0] == 0x52 && data[1] == 0x08 && data[2] == 0x01)
	{
		typedef struct
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

				rs = Util::CompareNumber(this->smoke, value1, value2, op);
				return true;
			}
		}
	}
	return false;
}

void ModuleSmoke::CheckTrigger()
{
	LOGD("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

void ModuleSmoke::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = idSmoke;
	dataValue["VALUE"] = smoke;
	jsonValue.append(dataValue);
	dataValue["ID"] = idPower;
	dataValue["VALUE"] = power;
	jsonValue.append(dataValue);
}

void ModuleSmoke::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_SMOKE] = smoke;
	jsonValue[KEY_ATTRIBUTE_SMOKE_PIN] = power;
}
