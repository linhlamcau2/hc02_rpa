#include "ModuleOnOff.h"
#include <Log.h>
#include <Util.h>
#include "Device.h"

ModuleOnOff::ModuleOnOff(Device *device) : Module(device)
{
	onoff = 0;
}

void ModuleOnOff::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct
	{
		uint8_t state;
		uint8_t onoff;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (len == 1)
		onoff = data_message->state;
	else
		onoff = data_message->onoff;
	BuildTelemetryValue(jsonValue);
	CheckTrigger();
}

bool ModuleOnOff::CheckData(Json::Value dataValue)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	bool rs = false;
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember("onoff") && dataValue["onoff"].isInt())
		{
			int onoff = dataValue["onoff"].asInt();
			rs = Util::CompareNumber(this->onoff, onoff, op);
		}
	}
	return rs;
}

void ModuleOnOff::CheckTrigger()
{
	LOGD("CheckTriggerOnOff");
	for (auto &sceneInputDevice : device->deviceSceneInputList)
	{
		if (CheckData(*sceneInputDevice->GetData()))
			sceneInputDevice->Trigger(true);
	}
}

void ModuleOnOff::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_THINGSBOARD
	jsonValue["onoff"] = onoff;
#else
	Json::Value dataValue;
	dataValue["ID"] = parameterToId["onoff"];
	dataValue["VALUE"] = onoff;
	jsonValue.append(dataValue);
#endif
}
