#include "ModuleDoorHangOn.h"
#include <Log.h>
#include <Util.h>
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleDoorHangOn::ModuleDoorHangOn(Device *device, uint32_t addr) : Module(device, addr)
{
	hangOn = 0;
	id = BLE_ATTRIBUTE_HANGON;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDoorHangOn::InitAttribute(int id, double value)
{
	if (this->id == id)
		hangOn = value;
}

void ModuleDoorHangOn::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, hangOn);
}
#endif

bool ModuleDoorHangOn::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x09 && data[2] == 0x04)
	{
		hangOn = data[3];
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return true;
	}
	return false;
}

bool ModuleDoorHangOn::CheckData(Json::Value &dataValue, bool &rs)
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
			uint16_t value1, value2;
			Json::Value listValue = dataValue["VALUE"];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				value1 = listValue[0].asInt();
				value2 = listValue[1].asInt();
			}
			string op = dataValue["OP"].asString();
			rs = Util::CompareNumber(this->hangOn, value1, value2, op);
			return true;
		}
	}
	return false;
}

// TODO: can nhac di chuyen den Module.cpp
void ModuleDoorHangOn::CheckTrigger()
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

void ModuleDoorHangOn::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = hangOn;
	jsonValue.append(dataValue);
}

