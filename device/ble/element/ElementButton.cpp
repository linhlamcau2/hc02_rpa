#include "ElementButton.h"
#include <Log.h>
#include <Util.h>
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementButton::ElementButton(Device *device, uint32_t addr) : Element(device, addr)
{
	bt = 0;
	id = BLE_ATTRIBUTE_BUTTON_1 + addr - device->GetAddr();
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ElementButton::InitAttribute(int id, double value)
{
	if (this->id == id)
		bt = value;
}

void ElementButton::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, bt);
}
#endif

bool ElementButton::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct
	{
		uint16_t opcode;
		uint8_t state;
		uint8_t bt;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == BLE_MESH_OPCODE_ONOFF)
	{
		if (len == 3)
			bt = data_message->state;
		else
			bt = data_message->bt;
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return true;
	}
	return false;
}

bool ElementButton::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
				dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
				dataValue.isMember("OP") && dataValue["OP"].isString())
		{
			// TODO: bug
			uint16_t bt = 0, mode = 0;
			Json::Value listValue = dataValue["VALUE"];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				bt = listValue[0].asInt();
				mode = listValue[1].asInt();
			}
			string op = dataValue["OP"].asString();
			rs = Util::CompareNumber(this->bt, bt, mode, op);
			return true;
		}
	}
	return false;
}

// TODO: can nhac di chuyen den Element.cpp
void ElementButton::CheckTrigger()
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

void ElementButton::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = bt;
	jsonValue.append(dataValue);
}

bool ElementButton::Do(Json::Value &dataValue)
{
	// LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			bleProtocol->SetOnOffLight(addr, value, 0, true);
			return true;
		}
	}
	return false;
}
