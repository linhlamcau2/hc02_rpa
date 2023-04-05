#include "ModulePirLight.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModulePirLight::ModulePirLight(Device *device, uint32_t addr) : Module(device, addr)
{
	pir = 0;
	lux = 0;
	idPir = BLE_ATTRIBUTE_PIR;
	idLux = BLE_ATTRIBUTE_LUX;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModulePirLight::InitAttribute(int id, double value)
{
	if (this->id == idPir)
		pir = value;
	else if (this->id == idLux)
		lux = value;
}

void ModulePirLight::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idPir, pir);
	database->DeviceAttributeAddOrReplace(device, idLux, lux);
}
#endif

int ModulePirLight::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idPir == id || this->idLux == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
			{
				if (this->idPir == id)
					pir = dataValue["VALUE"].asInt();
				else if (this->idLux == id)
					lux = dataValue["VALUE"].asInt();
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}

int ModulePirLight::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x05 && data[2] == 0x00)
	{
		typedef struct __attribute__((packed))
		{
			uint16_t pir;
			uint16_t scene;
			uint16_t lux;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)&data[3];
		pir = data_message->pir;
		lux = data_message->lux;
		if (lux > 0)
		{
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

bool ModulePirLight::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idPir == id || this->idLux == id)
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
					rs = Util::CompareNumber(this->pir, value1, value2, op);
					return true;
				}
			}
		}
	}
	return false;
}

// TODO: can nhac di chuyen den Module.cpp
void ModulePirLight::CheckTrigger()
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

void ModulePirLight::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = idPir;
	dataValue["VALUE"] = pir;
	jsonValue.append(dataValue);
	dataValue["ID"] = idLux;
	dataValue["VALUE"] = lux;
	jsonValue.append(dataValue);
}

void ModulePirLight::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_PIR] = pir;
	jsonValue[KEY_ATTRIBUTE_LUX] = lux;
}
