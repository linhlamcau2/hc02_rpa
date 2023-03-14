#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModulePinLevel.h"

using namespace std;

class DeviceBleSwitchScene6DC : public DeviceBle
{
private:
	ModuleButton *moduleButton[6];
	ModulePinLevel *modulePinLevel;

public:
	DeviceBleSwitchScene6DC(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);
};
