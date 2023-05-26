#include "DeviceBleSwitchOnoff.h"
#include "Log.h"

DeviceBleSwitchOnoff::DeviceBleSwitchOnoff(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite)
		: DeviceBle(id, name, mac, data, addr, type, version, isFavorite)
{
	moduleOnOff = new ModuleOnOff(this, addr);
	modules.push_back(moduleOnOff);
	powerSource = POWER_AC;
}
