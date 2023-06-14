#include "FunctionZone.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"

FunctionZone::FunctionZone(Device *device, string zoneId) : Function(device)
{
	this->zoneId = zoneId;
	zoneValue = 0;
}

FunctionZone::~FunctionZone()
{
}

int FunctionZone::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isMember(KEY_ATTRIBUTE_ZONE_ID) && dataValue[KEY_ATTRIBUTE_ZONE_ID].isString() &&
			dataValue.isMember(KEY_ATTRIBUTE_ZONE_VALUE) && dataValue[KEY_ATTRIBUTE_ZONE_VALUE].isInt())
	{
		string zoneId = dataValue[KEY_ATTRIBUTE_ZONE_ID].asString();
		int zoneValue = dataValue[KEY_ATTRIBUTE_ZONE_VALUE].asInt();
		if (this->zoneId == zoneId)
		{
			this->zoneValue = zoneValue;
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
		}
	}
	return CODE_ERROR;
}

bool FunctionZone::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_ZONE_ID) && dataValue[KEY_ATTRIBUTE_ZONE_ID].isString())
	{
		string zoneId = dataValue[KEY_ATTRIBUTE_ZONE_ID].asString();
		if (zoneId == this->zoneId)
		{
			if (dataValue.isMember("op") && dataValue["op"].isString() &&
					dataValue.isMember(KEY_ATTRIBUTE_ZONE_VALUE) && dataValue[KEY_ATTRIBUTE_ZONE_VALUE].isInt())
			{
				string op = dataValue["op"].asString();
				int value = dataValue[KEY_ATTRIBUTE_ZONE_VALUE].asInt();
				rs = Util::CompareNumber(op, this->zoneValue, value);
				return true;
			}
		}
	}
	return false;
}

void FunctionZone::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_ZONE_ID] = zoneId;
	jsonValue[KEY_ATTRIBUTE_ZONE_VALUE] = zoneValue;
}
