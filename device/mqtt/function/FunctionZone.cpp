#include "FunctionZone.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"

FunctionZone::FunctionZone(Device *device, uint32_t id) : Function(device)
{
	zone = 0;
	key = KEY_ATTRIBUTE_ZONE;
}

FunctionZone::~FunctionZone()
{
}

int FunctionZone::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	// TODO:
	// jsonValue = dataValue;
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
	// TODO:
	jsonValue[key] = zone;
}
