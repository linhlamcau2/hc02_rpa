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
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	ElementRgb *elementRgb[4];
#else
	ModuleOnOff *moduleOnOff;
	ModuleRgb *moduleRgb;
#endif

public:
	DeviceBleSwitchTouchRgb4(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
