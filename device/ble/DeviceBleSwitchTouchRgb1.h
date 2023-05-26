#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleSwitchTouchRgb1 : public DeviceBle
{
private:
	ElementButton *elementButton;
#ifndef CONFIG_USE_OLD_APP
	ElementRgb *elementRgb;
#endif

#ifdef CONFIG_USE_OLD_APP
	ModuleOnOff *moduleOnOff;
	ModuleRgb * moduleRgb;
#endif

public:
	DeviceBleSwitchTouchRgb1(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite);
};
