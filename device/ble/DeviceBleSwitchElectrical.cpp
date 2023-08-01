#include "DeviceBleSwitchElectrical.h"
#include "Log.h"

DeviceBleSwitchElectrical::DeviceBleSwitchElectrical(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version, uint8_t element)
		: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	countElement = element;
	for (int i = 0; i < element; i++)
	{
		moduleOnOff = new ModuleOnOff(this, addr + i, KEY_ATTRIBUTE_BUTTON, i);
		modules.push_back(moduleOnOff);
		moduleDimonDimoff = new ModuleDimonDimoff(this, addr + i, i);
		modules.push_back(moduleDimonDimoff);
	}
	powerSource = POWER_AC;
}
