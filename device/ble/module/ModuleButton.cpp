#include "ModuleButton.h"
#include <Log.h>
#include <Util.h>
#include "Device.h"
#include "Db.h"

ModuleButton::ModuleButton(Device *device, int index) : Module(device)
{
	bt = 0;
	this->index = index;
	buttonName = "bt" + to_string(index);
}

void ModuleButton::InitAttribute(int attributeId, double value)
{
	if (attributeId == parameterToId[buttonName])
		bt = value;
}

void ModuleButton::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, parameterToId[buttonName], bt);
}

void ModuleButton::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
	bt = data[0];
	SaveAttribute();
	BuildTelemetryValue(jsonValue);
	CheckTrigger();
}

bool ModuleButton::CheckData(Json::Value dataValue)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	bool rs = false;
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember(buttonName) && dataValue[buttonName].isInt())
		{
			int bt = dataValue[buttonName].asInt();
			rs = Util::CompareNumber(this->bt, bt, op);
		}
	}
	return rs;
}

void ModuleButton::CheckTrigger()
{
	LOGD("CheckTrigger");
	for (auto &sceneInputDevice : device->deviceSceneInputList)
	{
		if (CheckData(*sceneInputDevice->GetData()))
			sceneInputDevice->Trigger(true);
	}
}

void ModuleButton::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = parameterToId[buttonName];
	dataValue["VALUE"] = bt;
	jsonValue.append(dataValue);
}
