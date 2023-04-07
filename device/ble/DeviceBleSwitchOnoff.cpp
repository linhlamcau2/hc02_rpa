#include "DeviceBleSwitchOnoff.h"
#include "Log.h"

DeviceBleSwitchOnoff::DeviceBleSwitchOnoff(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr);
	powerSource = POWER_AC;
}
