#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"
#include "element/ElementOnOff.h"

using namespace std;

class DeviceBleSwitchTouchRgb3 : public DeviceBle
{
private:
	ElementButton *elementButton[3];
	ElementRgb *elementRgb[3];
	ElementOnOff * elementOnOff[3];

public:
	DeviceBleSwitchTouchRgb3(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
