#include "DeviceBleSwitchElectrical1.h"
#include "Log.h"

DeviceBleSwitchElectrical1::DeviceBleSwitchElectrical1(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite)
	: DeviceBle(id, name, mac, data, addr, type, version, isFavorite)
{

	elementButton = new ElementButton(this, addr);
	elements.push_back(elementButton);
#ifndef CONFIG_USE_OLD_APP
	elementRgb = new ElementRgb(this, addr);
	elements.push_back(elementRgb);
#endif
	powerSource = POWER_AC;

#ifdef CONFIG_USE_OLD_APP
	moduleOnOff = new ModuleOnOff(this, addr);
	modules.push_back(moduleOnOff);
	moduleDimonDimoff = new ModuleDimonDimoff(this, addr, 0);
	modules.push_back(moduleDimonDimoff);
#endif
}
