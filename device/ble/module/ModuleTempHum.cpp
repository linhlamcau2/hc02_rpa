#include "ModuleTempHum.h"
#include <byteswap.h>
#include <Log.h>
#include <Util.h>
#include "BleDefine.h"
#include "Device.h"

ModuleTempHum::ModuleTempHum(Device *device) : Module(device)
{
	temp = 0;
	hum = 0;
	idTemp = BLE_ATTRIBUTE_TEMP;
	idHum = BLE_ATTRIBUTE_HUMIDITY;
}

int ModuleTempHum::GetTemp()
{
	return temp;
}

void ModuleTempHum::SetTemp(int temp)
{
	this->temp = temp;
}

int ModuleTempHum::GetHum()
{
	return hum;
}

void ModuleTempHum::SetHum(int hum)
{
	this->hum = hum;
}

bool ModuleTempHum::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x06 && data[2] == 0x00)
	{
		typedef struct
		{
			uint16_t temp;
			uint16_t hum;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)&data[3];
		temp = bswap_16(data_message->temp);
		hum = bswap_16(data_message->hum);
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return true;
	}
	return false;
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
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt() &&
					dataValue.isMember("OP") && dataValue["OP"].isString())
			{
				uint16_t value = dataValue["VALUE"].asInt();
				string op = dataValue["OP"].asString();
				if (this->idTemp == id)
					rs = Util::CompareNumber(this->temp, value, op);
				else if (this->idHum == id)
					rs = Util::CompareNumber(this->hum, value, op);
				return true;
			}
		}
	}
	return false;
}

void ModuleTempHum::CheckTrigger()
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

void ModuleTempHum::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = parameterToId["temp"];
	dataValue["VALUE"] = temp;
	jsonValue.append(dataValue);
	dataValue["ID"] = parameterToId["hum"];
	dataValue["VALUE"] = hum;
	jsonValue.append(dataValue);
}
