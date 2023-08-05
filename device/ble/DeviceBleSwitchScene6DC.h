#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModuleBatteryLevel.h"

using namespace std;

class DeviceBleSwitchScene6DC : public DeviceBle
{
private:
	ModuleButton *moduleButton[6];
	ModuleBatteryLevel *moduleBatteryLevel;

public:
	DeviceBleSwitchScene6DC(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version);
};
