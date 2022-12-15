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
	for (auto &sceneInputDevice : device->deviceSceneInputList)
	{
		if (CheckData(*sceneInputDevice->GetData()))
			sceneInputDevice->Trigger(true);
	}
}

void ModulePinLevel::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_THINGSBOARD
	jsonValue["pin"] = pin;
#else
	Json::Value dataValue;
	dataValue["ID"] = parameterToId["pin"];
	dataValue["VALUE"] = pin;
	jsonValue.append(dataValue);
#endif
}
