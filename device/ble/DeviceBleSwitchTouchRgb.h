#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleRgb.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCallScene.h"

using namespace std;

class DeviceBleSwitchTouchRgb : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleOnOff *moduleOnOffAll;
	ModuleRgb *moduleRgb;
	ModuleCountDownSwitch *moduleCountDownSwitch;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCallScene *moduleCallScene;

public:
	DeviceBleSwitchTouchRgb(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement);
};
