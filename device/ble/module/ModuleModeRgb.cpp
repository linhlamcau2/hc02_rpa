#include "ModuleModeRgb.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleModeRgb::ModuleModeRgb(Device *device, uint32_t addr) : Module(device, addr)
{
	mode = 0;
	id = BLE_ATTRIBUTE_SCENE_RGB;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleModeRgb::InitAttribute(int id, double value)
{
	if (this->id == id)
		mode = value;
}

void ModuleModeRgb::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, mode);
}
#endif

// TODO: recheck
bool ModuleModeRgb::InputData(uint8_t *data, int len, Json::Value &jsonValue)
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

bool ModuleModeRgb::CheckData(Json::Value &dataValue, bool &rs)
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
			uint16_t mode1 = 0, mode2 = 0;
			string op = dataValue["OP"].asString();
			Json::Value listValue = dataValue["VALUE"];
			if (listValue.size() > 0)
			{
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					mode1 = listValue[0].asInt();
					mode2 = listValue[1].asInt();
				}
				else if (listValue.size() == 1 && listValue[0].isInt())
				{
					mode1 = listValue[0].asInt();
				}
				rs = Util::CompareNumber(this->mode, mode1, mode2, op);
				return true;
			}
		}
	}
	return false;
}

// TODO: can nhac di chuyen den Module.cpp
void ModuleModeRgb::CheckTrigger()
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

void ModuleModeRgb::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = mode;
	jsonValue.append(dataValue);
}

void ModuleModeRgb::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_MODE_RGB] = mode;
}

bool ModuleModeRgb::Do(Json::Value &dataValue)
{
	LOGD("Do data: %s", dataValue.toString().c_str());
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

bool ModuleModeRgb::DoV2(Json::Value &dataValue)
{
	LOGD("DoV2 data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && dataValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
	{
		int value = dataValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
		bleProtocol->CallModeRgb(addr, value);
		return true;
	}
	return false;
}
