#include "DeviceBleAll.h"
#include "Log.h"

DeviceBleAll::DeviceBleAll(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr);
	moduleDim = new ModuleDim(this, addr);
	modules.push_back(moduleOnOff);
	modules.push_back(moduleDim);
	powerSource = POWER_AC;
}
