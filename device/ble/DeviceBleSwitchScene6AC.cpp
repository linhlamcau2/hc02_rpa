#include "DeviceBleSwitchScene6AC.h"
#include "Log.h"

DeviceBleSwitchScene6AC::DeviceBleSwitchScene6AC(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, dataJson, addr, BLE_AC_SCENE_CONTACT, version)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i] = new ModuleButton(this, addr, i);
		modules.push_back(moduleButton[i]);
	}
}
