#include "ModuleButton.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleButton::ModuleButton(Device *device, uint32_t addr) : ModuleButton(device, addr, 0)
{
}

ModuleButton::ModuleButton(Device *device, uint32_t addr, int index) : Module(device, addr)
{
	bt = 0;
	id = BLE_ATTRIBUTE_BUTTON_1 + addr - device->GetAddr() + index;
	key = KEY_ATTRIBUTE_BUTTON + to_string(addr - device->GetAddr() + index);
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleButton::InitAttribute(int id, double value)
{
	if (this->id == id)
		bt = value;
}

void ModuleButton::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, bt);
}
#endif

int ModuleButton::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			bt = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

int ModuleButton::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x02 && data[2] == 0x00 && data[3] + 10 == id)
	{
		bt = data[4];
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleButton::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
				dataValue.isMember("OP") && dataValue["OP"].isString())
			{
				uint16_t bt = 0, mode = 0;
				string op = dataValue["OP"].asString();
				Json::Value listValue = dataValue["VALUE"];
				if (listValue.size() > 0)
				{
					if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
					{
						bt = listValue[0].asInt();
						mode = listValue[1].asInt();
					}
					else if (listValue.size() == 1 && listValue[0].isInt())
					{
						bt = listValue[0].asInt();
					}
					if (this->id == id)
						rs = Util::CompareNumber(this->bt, bt, mode, op);
					return true;
				}
			}
		}
	}
	return false;
}

// TODO: can nhac di chuyen den Module.cpp
void ModuleButton::CheckTrigger()
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

void ModuleButton::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = bt;
	jsonValue.append(dataValue);
}

void ModuleButton::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[key] = bt;
}

int ModuleButton::Do(Json::Value &dataValue)
{
	LOGD("ModuleButton Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			if (bleProtocol)
				bleProtocol->SetOnOffLight(addr, value, 0, true);
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

int ModuleButton::DoV2(Json::Value &dataValue)
{
	LOGV("DoV2 data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(key) && dataValue[key].isInt())
	{
		int bt = dataValue[key].asInt();
		if (bleProtocol)
		{
			if (bleProtocol->SetOnOffLight(addr, bt, 0, true) == CODE_OK)
			{
				this->bt = bt;
			}
		}
		else
			LOGW("BleProtocol null");
		return CODE_OK;
	}
	return CODE_ERROR;
}
