#include "ModuleTimeActionPir.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleTimeActionPir::ModuleTimeActionPir(Device *device, uint32_t addr) : Module(device, addr)
{
	time = 0;
	id = BLE_ATTRIBUTE_ACTIME;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleTimeActionPir::InitAttribute(int id, double value)
{
	if (this->id == id)
		time = value;
}

void ModuleTimeActionPir::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, time);
}
#endif

int ModuleTimeActionPir::InputData(Json::Value &dataValue, Json::Value &jsonValue, Json::Value &jsonValueV2)
{
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			time = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

int ModuleTimeActionPir::InputData(uint8_t *data, int len, Json::Value &jsonValue, Json::Value &jsonValueV2)
{
	if (data[0] == 0xe3 && data[1] == 0x11 && data[2] == 0x02 && data[3] == 0x45 && data[4] == 0x03)
	{
		time = data[5] | (data[6] << 8);
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleTimeActionPir::CheckData(Json::Value &dataValue, bool &rs)
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
				rs = Util::CompareNumber(this->time, value1, value2, op);
				return true;
			}
		}
	}
	return false;
}

void ModuleTimeActionPir::CheckTrigger()
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

void ModuleTimeActionPir::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = time;
	jsonValue.append(dataValue);
}

void ModuleTimeActionPir::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_ACTIME] = time;
}

int ModuleTimeActionPir::Do(Json::Value &dataValue)
{
	LOGV("ModuleTimeActionPir Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			if (bleProtocol)
			{
				bleProtocol->TimeActionPirLightSensor(addr, value);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

int ModuleTimeActionPir::DoV2(Json::Value &dataValue)
{
	LOGV("DoV2 data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_ACTIME) && dataValue[KEY_ATTRIBUTE_ACTIME].isInt())
	{
		int time = dataValue[KEY_ATTRIBUTE_ACTIME].asInt();
		if (bleProtocol->TimeActionPirLightSensor(addr, time) == CODE_OK)
		{
			this->time = time;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
