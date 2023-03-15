#include "DeviceBleSwitchTouchRgb1.h"
#include "Log.h"

DeviceBleSwitchTouchRgb1::DeviceBleSwitchTouchRgb1(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_SWITCH_RGB_1, version)
{
	elementButton = new ElementButton(this, addr);
	elementRgb = new ElementRgb(this, addr);
	elements.push_back(elementButton);
	elements.push_back(elementRgb);
	powerSource = POWER_AC;
}
