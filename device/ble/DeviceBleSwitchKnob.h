#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleCallScene.h"
#include "module/ModuleRgb.h"
using namespace std;

class DeviceBleSwitchKnob : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleOnOff *moduleOnOffAll;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCountDownSwitch *moduleCountDownSwitch;
	ModuleCallScene *moduleCallScene;
	ModuleRgb *moduleRgb;

public:
	DeviceBleSwitchKnob(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement);
};
