#include "ModuleTempHum.h"
#include <byteswap.h>
#include <Log.h>
#include <Util.h>
#include "Device.h"

ModuleTempHum::ModuleTempHum(Device *device) : Module(device)
{
	temp = 0;
	hum = 0;
}

float ModuleTempHum::GetTemp()
{
	return temp;
}

void ModuleTempHum::SetTemp(float temp)
{
	this->temp = temp;
}

float ModuleTempHum::GetHum()
{
	return hum;
}

void ModuleTempHum::SetHum(float hum)
{
	this->hum = hum;
}

void ModuleTempHum::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct
	{
		uint16_t temp;
		uint16_t hum;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	temp = bswap_16(data_message->temp) / 10;
	hum = bswap_16(data_message->hum) / 10;
	BuildTelemetryValue(jsonValue);
	CheckTrigger();
}

bool ModuleTempHum::CheckData(Json::Value dataValue)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	bool rs = false;
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember("temp") && dataValue["temp"].isInt())
		{
			int temp = dataValue["temp"].asInt();
			rs = Util::CompareNumber(this->temp, temp, op);
		}
		else if (dataValue.isMember("hum") && dataValue["hum"].isInt())
		{
			int hum = dataValue["hum"].asInt();
			rs = Util::CompareNumber(this->hum, hum, op);
		}
	}
	return rs;
}

void ModuleTempHum::CheckTrigger()
{
	LOGD("CheckTrigger");
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		if (CheckData(*ruleInputDevice->GetData()))
			ruleInputDevice->Trigger(true);
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
