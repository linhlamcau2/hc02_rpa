#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleSwitchTouchRgb : public DeviceBle
{
private:
	ModuleButton *moduleButton;
	ModuleOnOff *moduleOnOff;
	ModuleRgb *moduleRgb;

public:
	DeviceBleSwitchTouchRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
