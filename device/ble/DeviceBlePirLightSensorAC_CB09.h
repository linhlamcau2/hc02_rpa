#pragma once

#include "DeviceBle.h"
#include "module/ModulePirSensor.h"
#include "module/ModuleLightSensor.h"
#include "module/ModuleTimeActionPir.h"
#include "module/ModuleModeActionPir.h"
#include "module/ModulePirLight.h"
#include "module/ModuleOnOff.h"

using namespace std;

class DeviceBlePirLightSensorAC_CB09 : public DeviceBle
{
private:
	ModulePirLight *modulePirLight;
	ModulePirSensor *modulePirSensor;
	ModuleLightSensor *moduleLightSensor;
	ModuleTimeActionPir *moduleTimeActionPir;
    ModuleModeActionPir *moduleModeActionPir;
    ModuleOnOff *moduleOnOff;

public:
	DeviceBlePirLightSensorAC_CB09(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
