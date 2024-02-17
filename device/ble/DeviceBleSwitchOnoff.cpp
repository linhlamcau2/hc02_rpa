#include "DeviceBleSwitchOnoff.h"
#include "Log.h"

DeviceBleSwitchOnoff::DeviceBleSwitchOnoff(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_ONOFF);
	modules.push_back(moduleOnOff);
	moduleModeInput = new ModuleModeInput(this, addr);
	modules.push_back(moduleModeInput);
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);
	powerSource = POWER_AC;
}
