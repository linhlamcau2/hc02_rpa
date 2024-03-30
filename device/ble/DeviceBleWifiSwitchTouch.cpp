#include "DeviceBleWifiSwitchTouch.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleStatusStartup.h"
#include "Log.h"

DeviceBleWifiSwitchTouch::DeviceBleWifiSwitchTouch(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement)
	: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	ModuleOnOff *moduleOnOff;
	ModuleStatusStartup *moduleStatusStartup;
	this->countElement = countElement;
	for (int i = 0; i < countElement; i++)
	{
		moduleOnOff = new ModuleOnOff(this, addr + i, KEY_ATTRIBUTE_BUTTON, i);
		modules.push_back(moduleOnOff);
	}
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);
	powerSource = POWER_AC;
}
