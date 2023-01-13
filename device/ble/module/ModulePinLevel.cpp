#include "ModulePinLevel.h"
#include <byteswap.h>
#include <Log.h>
#include <Util.h>
#include "Device.h"

ModulePinLevel::ModulePinLevel(Device *device) : Module(device)
{
	pin = 0;
}

void ModulePinLevel::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
	pin = data[0];
	BuildTelemetryValue(jsonValue);
	CheckTrigger();
}

bool ModulePinLevel::CheckData(Json::Value dataValue)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	bool rs = false;
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember("pin") && dataValue["pin"].isInt())
		{
			int pin = dataValue["pin"].asInt();
			rs = Util::CompareNumber(this->pin, pin, op);
		}
	}
	return rs;
}

void ModulePinLevel::CheckTrigger()
{
	LOGD("CheckTrigger");
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		if (CheckData(*ruleInputDevice->GetData()))
			ruleInputDevice->Trigger(true);
	}
}

void ModulePinLevel::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = parameterToId["pin"];
	dataValue["VALUE"] = pin;
	jsonValue.append(dataValue);
}
