#include "ElementOnOff.h"
#include <Log.h>
#include <Util.h>
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementOnOff::ElementOnOff(Device *device, uint32_t addr) : Element(device, addr)
{
	onoff = 0;
	id = BLE_ATTRIBUTE_ONOFF;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ElementOnOff::InitAttribute(int id, double value)
{
	if (this->id == id)
		onoff = value;
}

void ElementOnOff::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, onoff);
}
#endif

bool ElementOnOff::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct
	{
		uint16_t opcode;
		uint8_t state;
		uint8_t onoff;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == BLE_MESH_OPCODE_ONOFF)
	{
		if (len == 3)
			onoff = data_message->state;
		else
			onoff = data_message->onoff;
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return true;
	}
	return false;
}

bool ElementOnOff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
				dataValue.isMember("VALUE") && dataValue["VALUE"].isInt() &&
				dataValue.isMember("OP") && dataValue["OP"].isString())
		{
			uint16_t onoff = dataValue["VALUE"].asInt();
			string op = dataValue["OP"].asString();
			rs = Util::CompareNumber(this->onoff, onoff, op);
			return true;
		}
	}
	return false;
}

// TODO: can nhac di chuyen den Element.cpp
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
	dataValue["ID"] = id;
	dataValue["VALUE"] = onoff;
	jsonValue.append(dataValue);
}

bool ElementOnOff::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
				dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			bleProtocol->SetOnOffLight(addr, value, 0, true);
			return true;
		}
	}
	return false;
}
