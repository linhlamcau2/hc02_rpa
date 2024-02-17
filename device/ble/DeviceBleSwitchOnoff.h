#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleModeInput.h"
#include "module/ModuleStatusStartup.h"

using namespace std;
class DeviceBleSwitchOnoff : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleModeInput *moduleModeInput;
	ModuleStatusStartup *moduleStatusStartup;

public:
	DeviceBleSwitchOnoff(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
