#include "DeviceBleSwitchScene6DC.h"

DeviceBleSwitchScene6DC::DeviceBleSwitchScene6DC(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i] = new ModuleButton(this, addr, i);
		modules.push_back(moduleButton[i]);
	}
	moduleBatteryLevel = new ModuleBatteryLevel(this, addr);
	modules.push_back(moduleBatteryLevel);
	powerSource = POWER_BATTERY;
}
