#include "DeviceBleWifiSwitchTouch.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCallScene.h"

DeviceBleWifiSwitchTouch::DeviceBleWifiSwitchTouch(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement)
	: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	ModuleOnOff *moduleOnOff;
	ModuleOnOff *moduleOnOffAll;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCallScene *moduleCallScene;

	this->countElement = countElement;
	for (int i = 0; i < countElement; i++)
	{
		moduleOnOff = new ModuleOnOff(this, addr + i, KEY_ATTRIBUTE_BUTTON, i);
		modules.push_back(moduleOnOff);
	}
	moduleOnOffAll = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_ONOFF);
	modules.push_back(moduleOnOffAll);
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);
	moduleCallScene = new ModuleCallScene(this, addr);
	modules.push_back(moduleCallScene);
	powerSource = POWER_AC;
}
