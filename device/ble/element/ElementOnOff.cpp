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

void ElementOnOff::InitAttribute(int attributeId, double value)
{
	if (attributeId == parameterToId[elementName])
		onoff = value;
}

void ElementOnOff::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, parameterToId[elementName], onoff);
}

bool ElementOnOff::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct
	{
		uint16_t opcode;
		uint8_t state;
		uint8_t onoff;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == 0x0482)
	{
		if (len == 3)
			onoff = data_message->state;
		else
			onoff = data_message->onoff;
		SaveAttribute();
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return true;
	}
	return false;
}

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

	SaveAttribute();
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
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

void ElementOnOff::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = parameterToId[elementName];
	dataValue["VALUE"] = onoff;
	jsonValue.append(dataValue);
}

bool ElementOnOff::Do(Json::Value &dataValue)
{
	// LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	int id = parameterToId[elementName];
	if (dataValue.isMember("ID") && dataValue["ID"].isInt() &&
			dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
	{
		int idJson = dataValue["ID"].asInt();
		if (idJson == id)
		{
			int value = dataValue["VALUE"].asInt();
			bleProtocol->SetOnOffLight(addr, value, 0, true);
			return true;
		}
	}
	return false;
}

bool ElementOnOff::Do(int value)
{
	LOGD("DoTrigger value: %d", value);
	// bleprotocol call setonoff light
	bleProtocol->SetOnOffLight(addr, value, 0, true);
	return true;
}
