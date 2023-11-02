#include "DeviceBleSwitchTouchRgb.h"
#include "Log.h"

DeviceBleSwitchTouchRgb::DeviceBleSwitchTouchRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, uint8_t element)
	: DeviceBle(id, name, mac, data, addr, type, version)
{
	this->element = element;
	for (int i = 0; i < this->element; i++)
	{
		moduleButton = new ModuleButton(this, addr + i);
		modules.push_back(moduleButton);
	}
	moduleOnOff = new ModuleOnOff(this, addr);
	modules.push_back(moduleOnOff);
	moduleRgb = new ModuleRgb(this, addr, 0);
	modules.push_back(moduleRgb);
	moduleCountDownSwitch = new ModuleCountDownSwitch(this, addr);
	modules.push_back(moduleCountDownSwitch);
	powerSource = POWER_AC;
}
