#include "FunctionZone.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"

FunctionZone::FunctionZone(Device *device, string id) : Function(device)
{
	this->id = id;
	zone = 0;
}

FunctionZone::~FunctionZone()
{
}

int FunctionZone::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isMember("id") && dataValue["id"].isString() &&
			dataValue.isMember("zone") && dataValue["zone"].isInt())
	{
		string id = dataValue["id"].asString();
		int zone = dataValue["zone"].asInt();
		if (this->id == id)
		{
			this->zone = zone;
			BuildTelemetryValue(jsonValue);
		}
	}
	return CODE_ERROR;
}

bool FunctionZone::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	// TODO:
	return false;
}

void FunctionZone::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_ZONE_ID] = id;
	jsonValue[KEY_ATTRIBUTE_ZONE_VALUE] = zone;
}
