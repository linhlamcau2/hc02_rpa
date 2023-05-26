#include "DeviceBleSwitchScene6AC.h"
#include "Log.h"

DeviceBleSwitchScene6AC::DeviceBleSwitchScene6AC(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite)
		: DeviceBle(id, name, mac, data, addr, BLE_AC_SCENE_CONTACT, version, isFavorite)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i] = new ModuleButton(this, addr, i);
		modules.push_back(moduleButton[i]);
	}
}
