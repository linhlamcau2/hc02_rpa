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
}

ModuleTimeActionPir::~ModuleTimeActionPir()
{
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

int ModuleTimeActionPir::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_ACTIME) && dataValue[KEY_ATTRIBUTE_ACTIME].isInt())
	{
		time = dataValue[KEY_ATTRIBUTE_ACTIME].asInt();
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleTimeActionPir::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0xe3 && data[1] == 0x11 && data[2] == 0x02 && data[3] == 0x45 && data[4] == 0x03)
	{
		time = data[5] | (data[6] << 8);
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleTimeActionPir::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_ACTIME) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_ACTIME].isInt())
		{
			int time = dataValue[KEY_ATTRIBUTE_ACTIME].asInt();
			rs = Util::CompareNumber(op, this->time, time);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_ACTIME].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_ACTIME];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int time1 = listValue[0].asInt();
				int time2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->time, time1, time2);
				return true;
			}
		}
	}
	return false;
}

void ModuleTimeActionPir::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_ACTIME] = time;
}

int ModuleTimeActionPir::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
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
