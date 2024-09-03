#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDimonDimoff.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleAllRelay.h"

using namespace std;

class DeviceBleSwitchElectrical : public DeviceBle
{
private:
	ModuleButton *moduleButton;
	ModuleOnOff *moduleOnOff;
	ModuleDimonDimoff *moduleDimonDimoff;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCountDownSwitch *moduleCountDownSwitch;
	ModuleAllRelay *moduleAllRelay;

public:
	DeviceBleSwitchElectrical(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, uint8_t element = 0);
	int BuildTelemetryValue(Json::Value &pushDataValue);
};
