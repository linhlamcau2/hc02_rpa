#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDimonDimoff.h"

using namespace std;

class DeviceBleSwitchElectrical3 : public DeviceBle
{
private:
	ElementButton *elementButton[3];
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	ElementRgb *elementRgb[3];
#else
	ModuleOnOff *moduleOnOff;
	ModuleDimonDimoff *moduleDimonDimoff;
#endif

public:
	DeviceBleSwitchElectrical3(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
