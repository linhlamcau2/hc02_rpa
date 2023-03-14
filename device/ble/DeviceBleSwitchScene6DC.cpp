#include "DeviceBleSwitchScene6DC.h"
#include "Log.h"

DeviceBleSwitchScene6DC::DeviceBleSwitchScene6DC(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_DC_SCENE_CONTACT, version)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i] = new ModuleButton(this, addr, i);
		modules.push_back(moduleButton[i]);
	}
	modulePinLevel = new ModulePinLevel(this, addr);
	modules.push_back(modulePinLevel);
	powerSource = POWER_BATTERY;
}
