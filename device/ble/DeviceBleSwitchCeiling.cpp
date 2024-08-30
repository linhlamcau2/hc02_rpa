#include "DeviceBleSwitchCeiling.h"

DeviceBleSwitchCeiling::DeviceBleSwitchCeiling(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version, uint8_t element)
	: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    ModuleCountDownSwitch *moduleCountDownSwitch;
    ModuleStatusStartup *moduleStatusStartup;
	ModuleOnOff *moduleOnOff;
	ModuleCallScene *moduleCallScene;

    this->countElement = element;
	moduleCountDownSwitch = new ModuleCountDownSwitch(this, addr);
	modules.push_back(moduleCountDownSwitch);
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);
	for (int i = 0; i < countElement; i++)
	{
		moduleOnOff = new ModuleOnOff(this, addr + i, KEY_ATTRIBUTE_BUTTON, i);
		modules.push_back(moduleOnOff);
	}
	moduleCallScene = new ModuleCallScene(this, addr);
	modules.push_back(moduleCallScene);
	powerSource = POWER_AC;
}