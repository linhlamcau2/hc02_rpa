#include "ModuleDoorStatus.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleDoorStatus::ModuleDoorStatus(Device *device, uint32_t addr) : Module(device, addr)
{
	status = 0;
}

ModuleDoorStatus::~ModuleDoorStatus()
{
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

int ModuleDoorStatus::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_DOOR) && dataValue[KEY_ATTRIBUTE_DOOR].isInt())
	{
		status = dataValue[KEY_ATTRIBUTE_DOOR].asInt();
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleDoorStatus::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x09 && data[2] == 0x00)
	{
		status = data[3];
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleDoorStatus::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_DOOR) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_DOOR].isInt())
		{
			int status = dataValue[KEY_ATTRIBUTE_DOOR].asInt();
			rs = Util::CompareNumber(op, this->status, status);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_DOOR].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_DOOR];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int status1 = listValue[0].asInt();
				int status2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->status, status1, status2);
				return true;
			}
		}
	}
	return false;
}

void ModuleDoorStatus::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_DOOR] = status;
}
