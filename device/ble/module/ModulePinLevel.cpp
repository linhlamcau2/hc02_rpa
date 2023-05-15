#include "ModulePinLevel.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModulePinLevel::ModulePinLevel(Device *device, uint32_t addr) : Module(device, addr)
{
	pin = 0;
	id = BLE_ATTRIBUTE_BATTERY;
}

ModulePinLevel::~ModulePinLevel()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModulePinLevel::InitAttribute(int id, double value)
{
	if (this->id == id)
		pin = value;
}

void ModulePinLevel::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, pin);
}
#endif

int ModulePinLevel::InputData(Json::Value &dataValue, Json::Value &jsonValue, Json::Value &jsonValueV2)
{
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			pin = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

int ModulePinLevel::InputData(uint8_t *data, int len, Json::Value &jsonValue, Json::Value &jsonValueV2)
{
	if (data[0] == 0x52 && data[1] == 0x01 && data[2] == 0x00)
	{
		pin = data[4];
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModulePinLevel::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
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
					if (this->id == id)
						rs = Util::CompareNumber(this->pin, value1, value2, op);
					return true;
				}
			}
		}
	}
	return false;
}

void ModulePinLevel::CheckTrigger()
{
	LOGV("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

void ModulePinLevel::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = pin;
	jsonValue.append(dataValue);
}

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
void ModulePinLevel::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_BATTERY] = pin;
}
#endif // CONFIG_USE_MESSAGE_FORMAT_V2
