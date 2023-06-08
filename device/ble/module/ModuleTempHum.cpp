#include "ModuleTempHum.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleTempHum::ModuleTempHum(Device *device, uint32_t addr) : Module(device, addr)
{
	temp = 0;
	hum = 0;
	idTemp = BLE_ATTRIBUTE_TEMP;
	idHum = BLE_ATTRIBUTE_HUMIDITY;
}

ModuleTempHum::~ModuleTempHum()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleTempHum::InitAttribute(int id, double value)
{
	if (this->id == idTemp)
		temp = value;
	else if (this->id == idHum)
		hum = value;
}

void ModuleTempHum::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idTemp, temp);
	database->DeviceAttributeAddOrReplace(device, idHum, hum);
}
#endif

int ModuleTempHum::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idTemp == id || this->idHum == id)
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
			{
				if (this->idTemp == id)
					temp = dataValue["VALUE"].asInt();
				else if (this->idHum == id)
					hum = dataValue["VALUE"].asInt();
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
	}
	return CODE_ERROR;
}

int ModuleTempHum::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	// if (data[0] == 0x52 && data[1] == 0x06 && data[2] == 0x00)
	// {
	// 	typedef struct
	// 	{
	// 		uint16_t temp;
	// 		uint16_t hum;
	// 	} data_message_t;
	// 	data_message_t *data_message = (data_message_t *)&data[3];
	// 	temp = bswap_16(data_message->temp);
	// 	hum = bswap_16(data_message->hum);
	// 	BuildTelemetryValue(jsonValue);
	// 	CheckTrigger();
	// 	return CODE_OK;
	// }
	// return CODE_ERROR;

	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint8_t value1[2];
		uint8_t value2[2];
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;

	if (data_message->opcode == 0x52)
	{
		if (data_message->header == 0x0006)
		{
			temp = (((data_message->value1[0] & 0x7F) << 8) | data_message->value1[1]) & 0x7FFF;
			if (data_message->value1[0] & 0x80)
				temp = (-1) * temp;
			hum = (data_message->value2[0] << 8) | data_message->value2[1];
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			Util::SetTempOfScreenTouch(temp);
			Util::SetHumOfScreenTouch(hum);
			return CODE_OK;
		}
		else if (data_message->header == 0x0107 && len >= 9)
		{
			hum = (data_message->value1[0] << 8) | data_message->value1[1];
			temp = (data[7] << 8) | data[8];
			if (data[5] == 0xff)
				temp = (-1) * temp;
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

bool ModuleTempHum::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idTemp == id || this->idHum == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
				dataValue.isMember("OP") && dataValue["OP"].isString())
			{
				uint16_t value1 = 0, value2 = 0;
				Json::Value listValue = dataValue["VALUE"];
				string op = dataValue["OP"].asString();
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
					if (this->idTemp == id)
						rs = Util::CompareNumber(this->temp / 10, value1, value2, op);
					else if (this->idHum == id)
						rs = Util::CompareNumber(this->hum / 10, value1, value2, op);
					return true;
				}
			}
		}
	}
	return false;
}

void ModuleTempHum::CheckTrigger()
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

void ModuleTempHum::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_TEMP] = temp;
	jsonValue[KEY_ATTRIBUTE_HUMIDITY] = hum;
#else
	Json::Value dataValue;
	dataValue["ID"] = idTemp;
	dataValue["VALUE"] = temp;
	jsonValue.append(dataValue);
	dataValue["ID"] = idHum;
	dataValue["VALUE"] = hum;
	jsonValue.append(dataValue);
#endif
}
