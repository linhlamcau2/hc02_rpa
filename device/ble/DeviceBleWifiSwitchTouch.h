#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCallScene.h"
#include "module/ModuleButtonAll.h"

using namespace std;

class DeviceBleWifiSwitchTouch : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleOnOff *moduleOnOffAll;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCallScene *moduleCallScene;
	ModuleButtonAll *moduleButtonAll;

public:
	DeviceBleWifiSwitchTouch(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement);
};
