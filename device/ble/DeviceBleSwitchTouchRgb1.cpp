#include "DeviceBleSwitchTouchRgb1.h"
#include "Log.h"

DeviceBleSwitchTouchRgb1::DeviceBleSwitchTouchRgb1(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, data, addr, type, version)
{

	elementButton = new ElementButton(this, addr);
	elementRgb = new ElementRgb(this, addr);
	elementOnOff = new ElementOnOff(this, addr);
	elements.push_back(elementButton);
	elements.push_back(elementRgb);
	elements.push_back(elementOnOff);
	powerSource = POWER_AC;
}
