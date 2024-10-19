#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleRgb.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleStatusStartup.h"

using namespace std;

class DeviceBleSocketSwitch : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleRgb *moduleRgb;
	ModuleCountDownSwitch *moduleCountDownSwitch;
	ModuleStatusStartup *moduleStatusStartup;

public:
	DeviceBleSocketSwitch(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement);
};
