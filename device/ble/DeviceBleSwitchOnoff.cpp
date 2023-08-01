#include "DeviceBleSwitchOnoff.h"
#include "Log.h"

DeviceBleSwitchOnoff::DeviceBleSwitchOnoff(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_BUTTON);
	modules.push_back(moduleOnOff);
	powerSource = POWER_AC;
}
