#pragma once

#include "DeviceBle.h"
#include "module/ModulePirSensor.h"
#include "module/ModuleLightSensor.h"
#include "module/ModulePinLevel.h"
#include "module/ModuleTimeActionPir.h"
#include "module/ModulePirLight.h"
#include <mutex>

using namespace std;

class DeviceBlePirLightSensorDC : public DeviceBle
{
private:
	ModulePirLight * modulePirLight;
	ModulePirSensor *modulePirSensor;
	ModuleLightSensor *moduleLightSensor;
	ModulePinLevel *modulePinLevel;
	ModuleTimeActionPir *moduleTimeActionPir;

public:
	DeviceBlePirLightSensorDC(string id, string name, string mac, string data, uint32_t addr, uint16_t version, bool isFavorite);
};
