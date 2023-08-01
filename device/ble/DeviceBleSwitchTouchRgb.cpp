#include "DeviceBleSwitchTouchRgb.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleRgb.h"
#include "Log.h"

DeviceBleSwitchTouchRgb::DeviceBleSwitchTouchRgb(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version, uint8_t element)
		: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	ModuleOnOff *moduleOnOff;
	ModuleRgb *moduleRgb;
	countElement = element;
	for (int i = 0; i < element; i++)
	{
		moduleOnOff = new ModuleOnOff(this, addr + i, KEY_ATTRIBUTE_BUTTON, i);
		modules.push_back(moduleOnOff);
		moduleRgb = new ModuleRgb(this, addr + i, i);
		modules.push_back(moduleRgb);
	}
	powerSource = POWER_AC;
}
