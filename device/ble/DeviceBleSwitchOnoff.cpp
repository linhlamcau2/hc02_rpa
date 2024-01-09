#include "DeviceBleSwitchOnoff.h"
#include "Log.h"

DeviceBleSwitchOnoff::DeviceBleSwitchOnoff(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr);
	modules.push_back(moduleOnOff);
	moduleModeInput = new ModuleModeInput(this, addr);
	modules.push_back(moduleModeInput);
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);
	powerSource = POWER_AC;
}
