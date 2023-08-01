#pragma once

#include "DeviceBle.h"
#include "module/ModulePirSensor.h"
#include "module/ModuleLightSensor.h"
#include "module/ModuleTimeActionPir.h"
#include "module/ModulePirLight.h"

using namespace std;

class DeviceBlePirLightSensorAC : public DeviceBle
{
private:
	ModulePirLight *modulePirLight;
	ModulePirSensor *modulePirSensor;
	ModuleLightSensor *moduleLightSensor;
	ModuleTimeActionPir *moduleTimeActionPir;

public:
	DeviceBlePirLightSensorAC(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint16_t version);
};
