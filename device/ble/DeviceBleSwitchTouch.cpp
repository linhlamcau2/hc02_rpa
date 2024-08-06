#include "DeviceBleSwitchTouch.h"
#include "module/ModuleRelaySwitch.h"

DeviceBleSwitchTouch::DeviceBleSwitchTouch(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t numRelay)
		: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	ModuleRelaySwitch *moduleRelaySwitch;
	for (int i = 0; i < numRelay; i++)
	{
		moduleRelaySwitch = new ModuleRelaySwitch(this, addr, i);
		modules.push_back(moduleRelaySwitch);
	}
	powerSource = POWER_AC;
}
