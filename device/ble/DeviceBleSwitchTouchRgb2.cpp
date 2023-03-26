#include "DeviceBleSwitchTouchRgb2.h"
#include "Log.h"

DeviceBleSwitchTouchRgb2::DeviceBleSwitchTouchRgb2(string id, string name, string mac, string data, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, BLE_SWITCH_RGB_2, version)
{
	for (int i = 0; i < 2; i++)
	{
		elementButton[i] = new ElementButton(this, addr + i);
		elementRgb[i] = new ElementRgb(this, addr + i);
		elements.push_back(elementButton[i]);
		elements.push_back(elementRgb[i]);
	}
	countElement = 2;
	powerSource = POWER_AC;
}
