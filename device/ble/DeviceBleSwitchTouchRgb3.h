#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleSwitchTouchRgb3 : public DeviceBle
{
private:
	ElementButton *elementButton[3];
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	ElementRgb *elementRgb[3];
#else
	ModuleOnOff *moduleOnOff;
	ModuleRgb *moduleRgb;
#endif

public:
	DeviceBleSwitchTouchRgb3(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
