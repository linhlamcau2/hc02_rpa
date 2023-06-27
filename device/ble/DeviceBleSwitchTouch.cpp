#include "DeviceBleSwitchTouch.h"
#include "Log.h"

DeviceBleSwitchTouch::DeviceBleSwitchTouch(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, uint8_t numRelay)
	: DeviceBle(id, name, mac, data, addr, type, version)
{
	for (int i = 0; i < numRelay; i++)
	{
		moduleRelaySwitch = new ModuleRelaySwitch(this, addr, i);
		modules.push_back(moduleRelaySwitch);
	}
	powerSource = POWER_AC;
}
