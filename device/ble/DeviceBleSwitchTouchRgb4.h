#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleSwitchTouchRgb4 : public DeviceBle
{
private:
	ElementButton *elementButton[4];
#ifndef CONFIG_USE_OLD_APP
	ElementRgb *elementRgb[4];
#endif

#ifdef CONFIG_USE_OLD_APP
	ModuleOnOff *moduleOnOff;
	ModuleRgb *moduleRgb;
#endif

public:
	DeviceBleSwitchTouchRgb4(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
