#include "DeviceBleSwitchScene6DC.h"
#include "Log.h"

DeviceBleSwitchScene6DC::DeviceBleSwitchScene6DC(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, type, version)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i] = new ModuleButton(this, addr, i);
		modules.push_back(moduleButton[i]);
	}
	modulePinLevel = new ModulePinLevel(this, addr);
	modules.push_back(modulePinLevel);
	powerSource = POWER_BATTERY;
}
