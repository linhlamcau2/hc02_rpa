#include "DeviceBleLightOnoffCctDim.h"
#include "Log.h"

DeviceBleLightOnoffCctDim::DeviceBleLightOnoffCctDim(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, data, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr);
	moduleDim = new ModuleDim(this, addr);
	moduleCallScene = new ModuleCallScene(this, addr);
	elementCct = new ElementCct(this, addr + 1);
	modules.push_back(moduleOnOff);
	modules.push_back(moduleDim);
	modules.push_back(moduleCallScene);
	elements.push_back(elementCct);
	countElement = 2;
	powerSource = POWER_AC;
}
