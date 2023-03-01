#include "ModuleDoorStatus.h"
#include <Log.h>
#include <Util.h>
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleDoorStatus::ModuleDoorStatus(Device *device, uint32_t addr) : Module(device, addr)
{
	status = 0;
	id = BLE_ATTRIBUTE_DOOR;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDoorStatus::InitAttribute(int id, double value)
{
	if (this->id == id)
		status = value;
}

void ModuleDoorStatus::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, status);
}
#endif

bool ModuleDoorStatus::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x09 && data[2] == 0x00)
	{
		status = data[3];
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return true;
	}
	return false;
}

bool ModuleDoorStatus::CheckData(Json::Value &dataValue, bool &rs)
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
			rs = Util::CompareNumber(this->status, value1, value2, op);
			return true;
		}
	}
	return false;
}

// TODO: can nhac di chuyen den Module.cpp
void ModuleDoorStatus::CheckTrigger()
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

void ModuleDoorStatus::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = status;
	jsonValue.append(dataValue);
}

