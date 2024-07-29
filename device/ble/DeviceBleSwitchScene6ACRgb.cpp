#include "DeviceBleSwitchScene6ACRgb.h"

DeviceBleSwitchScene6ACRgb::DeviceBleSwitchScene6ACRgb(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	for (int i = 0; i < 6; i++)
	{
		moduleRgb[i] = new ModuleRgb(this, addr, i);
		modules.push_back(moduleRgb[i]);
		moduleButton[i] = new ModuleButton(this, addr, i);
		modules.push_back(moduleButton[i]);
	}
}
