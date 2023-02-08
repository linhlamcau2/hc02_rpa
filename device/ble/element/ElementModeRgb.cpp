#include "ElementModeRgb.h"
#include <Log.h>
#include <Util.h>
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementModeRgb::ElementModeRgb(Device *device, uint32_t addr) : Element(device, addr)
{
	mode = 0;
	id = BLE_ATTRIBUTE_SCENE_RGB;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ElementModeRgb::InitAttribute(int id, double value)
{
	if (this->id == id)
		mode = value;
}

void ElementModeRgb::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, mode);
}
#endif

// TODO: recheck
bool ElementModeRgb::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct
	{
		uint16_t opcode;
		uint16_t reverse;
		uint16_t idScene;
		uint16_t magic;
		uint8_t mode;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == BLE_MESH_OPCODE_RGB)
	{
		if (data_message->idScene == 0)
		{
			mode = data_message->mode;
			// TODO: recheck
			if (1 <= mode && mode <= 6)
			{
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute();
#endif
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return true;
			}
		}
	}
	return false;
}

bool ElementModeRgb::CheckData(Json::Value &dataValue, bool &rs)
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
			uint16_t mode = dataValue["VALUE"].asInt();
			string op = dataValue["OP"].asString();
			rs = Util::CompareNumber(this->mode, mode, op);
			return true;
		}
	}
	return false;
}

// TODO: can nhac di chuyen den Element.cpp
void ElementModeRgb::CheckTrigger()
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

void ElementModeRgb::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = mode;
	jsonValue.append(dataValue);
}

bool ElementModeRgb::Do(Json::Value &dataValue)
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
			bleProtocol->CallModeRgb(addr, value);
			return true;
		}
	}
	return false;
}
