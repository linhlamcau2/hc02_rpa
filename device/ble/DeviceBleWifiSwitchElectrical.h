#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDimonDimoff.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleCallScene.h"
#include "module/ModuleButtonAll.h"

using namespace std;

class DeviceBleWifiSwitchElectrical : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleOnOff *moduleOnOffAll;
	ModuleDimonDimoff *moduleDimonDimoff;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCallScene *moduleCallScene;
	ModuleButtonAll *moduleButtonAll;

public:
	DeviceBleWifiSwitchElectrical(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement);
};
