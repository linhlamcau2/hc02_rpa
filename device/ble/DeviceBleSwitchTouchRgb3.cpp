#include "DeviceBleSwitchTouchRgb3.h"
#include "Log.h"

DeviceBleSwitchTouchRgb3::DeviceBleSwitchTouchRgb3(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_SWITCH_RGB_3, version)
{
	for (int i = 0; i < 3; i++)
	{
		elementButton[i] = new ElementButton(this, addr + i);
		elementRgb[i] = new ElementRgb(this, addr + i);
		elements.push_back(elementButton[i]);
		elements.push_back(elementRgb[i]);
	}
	countElement = 3;
	powerSource = POWER_AC;
}
