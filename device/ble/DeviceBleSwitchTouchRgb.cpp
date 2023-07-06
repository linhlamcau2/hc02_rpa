#include "DeviceBleSwitchTouchRgb.h"
#include "Log.h"

DeviceBleSwitchTouchRgb::DeviceBleSwitchTouchRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, uint8_t element)
	: DeviceBle(id, name, mac, data, addr, type, version)
{
	for (int i = 0; i < element; i++)
	{
		moduleButton = new ModuleButton(this, addr + i);
		modules.push_back(moduleButton);
	}
	moduleOnOff = new ModuleOnOff(this, addr);
	modules.push_back(moduleOnOff);
	moduleRgb = new ModuleRgb(this, addr);
	modules.push_back(moduleRgb);
	powerSource = POWER_AC;
}
