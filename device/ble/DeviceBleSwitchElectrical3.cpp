#include "DeviceBleSwitchElectrical3.h"
#include "Log.h"

DeviceBleSwitchElectrical3::DeviceBleSwitchElectrical3(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, data, addr, type, version)
{
	for (int i = 0; i < 3; i++)
	{
		elementButton[i] = new ElementButton(this, addr + i);
		elements.push_back(elementButton[i]);
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
		elementRgb[i] = new ElementRgb(this, addr + i);
		elements.push_back(elementRgb[i]);
#endif
	}
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	countElement = 3;
#endif

#ifndef CONFIG_USE_MESSAGE_FORMAT_V2
	moduleOnOff = new ModuleOnOff(this, addr);
	modules.push_back(moduleOnOff);
	moduleDimonDimoff = new ModuleDimonDimoff(this, addr, 0);
	modules.push_back(moduleDimonDimoff);
#endif

	powerSource = POWER_AC;
}
