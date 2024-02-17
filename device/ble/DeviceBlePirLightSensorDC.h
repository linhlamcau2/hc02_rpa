#pragma once

#include "DeviceBle.h"
#include "module/ModulePirSensor.h"
#include "module/ModuleLightSensor.h"
#include "module/ModuleBatteryLevel.h"
#include "module/ModuleTimeActionPir.h"
#include "module/ModulePirLight.h"
#include "module/ModuleSensiPir.h"

using namespace std;

class DeviceBlePirLightSensorDC : public DeviceBle
{
private:
	ModulePirLight *modulePirLight;
	ModulePirSensor *modulePirSensor;
	ModuleLightSensor *moduleLightSensor;
	ModuleBatteryLevel *moduleBatteryLevel;
	ModuleTimeActionPir *moduleTimeActionPir;
	ModuleSensiPir *moduleSensiPir;

public:
	DeviceBlePirLightSensorDC(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
