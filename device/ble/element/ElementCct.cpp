#include "ElementCct.h"
#include <Log.h>
#include <Util.h>
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementCct::ElementCct(Device *device, uint32_t addr) : Element(device, addr)
{
	cct = 0;
    elementName = "cct";
}

void ElementCct::InitAttribute(int attributeId, double value)
{
	if (attributeId == parameterToId[elementName])
		cct = value;
}

void ElementCct::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, parameterToId[elementName], cct);
}

void ElementCct::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct
	{
		uint16_t cct_first;
		uint16_t magic;
        uint16_t cct;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (len  <= 4)
		cct = data_message->cct_first;
	else
		cct = data_message->cct;

	SaveAttribute();
	BuildTelemetryValue(jsonValue);
	CheckTrigger();
}

bool ElementCct::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember(elementName) && dataValue[elementName].isInt())
		{
			uint16_t cct = dataValue[elementName].asInt();
			rs = Util::CompareNumber(this->cct, cct, op);
			return true;
		}
	}
	return false;
}

void ElementCct::CheckTrigger()
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

static int Para2PercentCct(uint16_t para)
{
    return ((para - 800) / 192);
}
void ElementCct::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = parameterToId[elementName];
	dataValue["VALUE"] = Para2PercentCct(cct);
	jsonValue.append(dataValue);
}

bool ElementCct::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	if (dataValue.isMember(elementName) && dataValue[elementName].isInt())
	{
	}
	return false;
}

bool ElementCct::Do(int value)
{
	LOGD("DoTrigger value: %d", value);
	//bleprotocol call setonoff light
	bleProtocol->SetCctLight(addr, value, 0, true);
	return true;
}
