#include "ModuleOnOff.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleOnOff::ModuleOnOff(Device *device, uint32_t addr) : Module(device, addr)
{
	onoff = 0;
	id = BLE_ATTRIBUTE_ONOFF;
	code = KEY_ATTRIBUTE_ONOFF;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleOnOff::InitAttribute(int id, double value)
{
	if (this->id == id)
		onoff = value;
}

void ModuleOnOff::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, onoff);
}
#endif

int ModuleOnOff::InputData(Json::Value &dataValue, Json::Value &jsonValue, Json::Value &jsonValue2)
{
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			onoff = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

int ModuleOnOff::InputData(uint8_t *data, int len, Json::Value &jsonValue, Json::Value &jsonValueV2)
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
		{
			onoff = data_message->state;
		}
		else
		{
			onoff = data_message->onoff;
		}
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		BuildTelemetryValue(jsonValue);
		BuildTelemetryValueV2(jsonValueV2);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleOnOff::CheckData(Json::Value &dataValue, bool &rs)
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
			uint16_t value1 = 0, value2 = 0;
			string op = dataValue["OP"].asString();
			Json::Value listValue = dataValue["VALUE"];
			if (listValue.size() > 0)
			{
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					value1 = listValue[0].asInt();
					value2 = listValue[1].asInt();
				}
				else if (listValue.size() == 1 && listValue[0].isInt())
				{
					value1 = listValue[0].asInt();
				}
				rs = Util::CompareNumber(this->onoff, value1, value2, op);
				return true;
			}
		}
	}
	return false;
}

bool ModuleOnOff::CheckDataV2(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("op") && dataValue["op"].isString() &&
		dataValue.isMember(KEY_ATTRIBUTE_ONOFF) && dataValue[KEY_ATTRIBUTE_ONOFF].isString())
	{
		int value = dataValue[KEY_ATTRIBUTE_ONOFF].asInt();
		string op = dataValue["op"].asString();
		rs = Util::CompareNumber(this->onoff, value, value, op);
		return true;
	}
	return false;
}

// TODO: can nhac di chuyen den Module.cpp
void ModuleOnOff::CheckTrigger()
{
	LOGV("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
		else if (CheckDataV2(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

void ModuleOnOff::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = onoff;
	jsonValue.append(dataValue);
}

void ModuleOnOff::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_ONOFF] = onoff;
}

int ModuleOnOff::Do(Json::Value &dataValue)
{
	LOGD("ModuleOnOff Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
			dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			if (bleProtocol)
			{
				bleProtocol->SetOnOffLight(addr, value, 0, true);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

int ModuleOnOff::DoV2(Json::Value &dataValue)
{
	LOGV("DoV2 data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_ONOFF) && dataValue[KEY_ATTRIBUTE_ONOFF].isInt())
	{
		int onoff = dataValue[KEY_ATTRIBUTE_ONOFF].asInt();
		if (bleProtocol->SetOnOffLight(addr, onoff, 0, true) == CODE_OK)
		{
			this->onoff = onoff;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
