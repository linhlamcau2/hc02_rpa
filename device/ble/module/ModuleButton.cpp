#include "ModuleButton.h"
#include <Log.h>
#include <Util.h>
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleButton::ModuleButton(Device *device, int index) : Module(device)
{
	bt = 0;
	id = BLE_ATTRIBUTE_BUTTON_1 + index;
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

bool ModuleButton::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x02 && data[2] == 0x00 && data[3] + 10 == id)
	{
		bt = data[4];
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return true;
	}
	return false;
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
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt() &&
					dataValue.isMember("OP") && dataValue["OP"].isString())
			{
				uint16_t value = dataValue["VALUE"].asInt();
				string op = dataValue["OP"].asString();
				if (this->id == id)
					rs = Util::CompareNumber(this->bt, value, op);
				return true;
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
