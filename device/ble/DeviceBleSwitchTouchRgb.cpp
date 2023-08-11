#include "DeviceBleSwitchTouchRgb.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleRgb.h"
#include "module/ModuleButton.h"
#include "Log.h"

DeviceBleSwitchTouchRgb::DeviceBleSwitchTouchRgb(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version, uint8_t countElement)
		: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	ModuleOnOff *moduleOnOff;
	ModuleRgb *moduleRgb;
	ModuleButton *moduleButton;
	this->countElement = countElement;
	for (int i = 0; i < countElement; i++)
	{
		moduleButton = new ModuleButton(this, addr + i, i);
		modules.push_back(moduleButton);
		moduleRgb = new ModuleRgb(this, addr + i, i);
		modules.push_back(moduleRgb);
	}
	// moduleDimonDimoff = new ModuleDimonDimoff(this, addr);
	// modules.push_back(moduleDimonDimoff);
	// moduleRgb = new ModuleRgb(this, addr);
	// modules.push_back(moduleRgb);
	powerSource = POWER_AC;
}
