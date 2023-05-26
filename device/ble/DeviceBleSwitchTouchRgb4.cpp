#include "DeviceBleSwitchTouchRgb4.h"
#include "Log.h"

DeviceBleSwitchTouchRgb4::DeviceBleSwitchTouchRgb4(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite)
	: DeviceBle(id, name, mac, data, addr, type, version, isFavorite)
{
	for (int i = 0; i < 4; i++)
	{
		elementButton[i] = new ElementButton(this, addr + i);
		elements.push_back(elementButton[i]);
#ifndef CONFIG_USE_OLD_APP
		elementRgb[i] = new ElementRgb(this, addr + i);
		elements.push_back(elementRgb[i]);
#endif
	}
#ifndef CONFIG_USE_OLD_APP
	countElement = 4;
#endif
	powerSource = POWER_AC;

#ifdef CONFIG_USE_OLD_APP
	moduleOnOff = new ModuleOnOff(this, addr);
	modules.push_back(moduleOnOff);
	moduleRgb = new ModuleRgb(this, addr, 0);
	modules.push_back(moduleRgb);
#endif
}
