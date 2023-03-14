#include "DeviceBleSwitchTouchRgb4.h"
#include "Log.h"

DeviceBleSwitchTouchRgb4::DeviceBleSwitchTouchRgb4(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_SWITCH_RGB_4, version)
{
	for (int i = 0; i < 4; i++)
	{
		elementButton[i] = new ElementButton(this, addr + i);
		elementRgb[i] = new ElementRgb(this, addr + i);
		elements.push_back(elementButton[i]);
		elements.push_back(elementRgb[i]);
	}
	// TODO: recheck 4 or 5
	countElement = 4;
	powerSource = POWER_AC;
}
