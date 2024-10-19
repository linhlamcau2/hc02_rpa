#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDimonDimoff.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleCallScene.h"

using namespace std;

class DeviceBleSwitchElectrical : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleOnOff *moduleOnOffAll;
	ModuleDimonDimoff *moduleDimonDimoff;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCountDownSwitch *moduleCountDownSwitch;
	ModuleCallScene *moduleCallScene;

public:
	DeviceBleSwitchElectrical(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement);
};
