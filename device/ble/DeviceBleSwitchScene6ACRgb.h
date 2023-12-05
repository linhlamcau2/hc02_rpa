#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleSwitchScene6ACRgb : public DeviceBle
{
private:
	ModuleRgb *moduleRgb[6];
	ModuleButton *moduleButton[6];

public:
	DeviceBleSwitchScene6ACRgb(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
