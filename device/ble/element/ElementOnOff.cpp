#include "ElementOnOff.h"
#include <Log.h>
#include <Util.h>
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementOnOff::ElementOnOff(Device *device, uint32_t addr) : Element(device, addr)
{
	onoff = 0;
	elementName = "onoff" + to_string(addr - device->GetAddr());
}

#ifdef CONFIG_FPT_SERVER
void ElementOnOff::InitAttribute(int attributeId, double value)
{
	if (attributeId == parameterToId[elementName])
		onoff = value;
}

void ElementOnOff::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, parameterToId[elementName], onoff);
}
#endif

void ElementOnOff::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
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

#ifdef CONFIG_FPT_SERVER
	SaveAttribute();
#endif
	BuildTelemetryValue(jsonValue);
	CheckTrigger();
}

bool ElementOnOff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember(elementName) && dataValue[elementName].isInt())
		{
			int onoff = dataValue[elementName].asInt();
			rs = Util::CompareNumber(this->onoff, onoff, op);
			return true;
		}
	}
	return false;
}

void ElementOnOff::CheckTrigger()
{
	LOGD("CheckTrigger");
	bool rs;
	for (auto &sceneInputDevice : device->deviceSceneInputList)
	{
		rs = false;
		if (CheckData(*sceneInputDevice->GetData(), rs))
			sceneInputDevice->Trigger(rs);
	}
}

void ElementOnOff::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_THINGSBOARD
	jsonValue[elementName] = onoff;
#else
	Json::Value dataValue;
	dataValue["ID"] = parameterToId[elementName];
	dataValue["VALUE"] = onoff;
	jsonValue.append(dataValue);
#endif
}

bool ElementOnOff::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	if (dataValue.isMember(elementName) && dataValue[elementName].isInt())
	{
		int onoff = dataValue[elementName].asInt();
		if (onoff == 0 || onoff == 1)
		{
			bleProtocol->TurnOnOff(addr, onoff);
			return true;
		}
		else if (onoff == 2)
		{
			bleProtocol->TurnOnOff(addr, this->onoff ? 0 : 1);
			return true;
		}
	}
	return false;
}

bool ElementOnOff::Do(int value)
{
	LOGD("DoTrigger value: %d", value);
	bleProtocol->TurnOnOff(addr, value);
	return true;
}
