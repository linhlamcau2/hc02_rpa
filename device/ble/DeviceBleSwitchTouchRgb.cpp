#include "DeviceBleSwitchTouchRgb.h"
#include "Log.h"

DeviceBleSwitchTouchRgb::DeviceBleSwitchTouchRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, type, version)
{
	moduleButton = new ModuleButton(this, addr);
	modules.push_back(moduleButton);
	moduleOnOff = new ModuleOnOff(this, addr);
	modules.push_back(moduleOnOff);
	moduleRgb = new ModuleRgb(this, addr, 0);
	modules.push_back(moduleRgb);
	powerSource = POWER_AC;
}
