#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleSwitchTouchRgb2 : public DeviceBle
{
private:
	ElementButton *elementButton[2];
#ifndef CONFIG_USE_OLD_APP
	ElementRgb *elementRgb[2];
#endif

#ifdef CONFIG_USE_OLD_APP
	ModuleOnOff *moduleOnOff;
	ModuleRgb *moduleRgb;
#endif

public:
	DeviceBleSwitchTouchRgb2(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite);
};
