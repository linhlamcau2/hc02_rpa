#include "DeviceBleSwitchElectrical.h"
#include "Log.h"

DeviceBleSwitchElectrical::DeviceBleSwitchElectrical(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, type, version)
{
	moduleButton = new ModuleButton(this, addr);
	modules.push_back(moduleButton);
	moduleOnOff = new ModuleOnOff(this, addr);
	modules.push_back(moduleOnOff);
	moduleDimonDimoff = new ModuleDimonDimoff(this, addr, 0);
	modules.push_back(moduleDimonDimoff);
	powerSource = POWER_AC;
}
