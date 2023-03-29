#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"
#include "element/ElementOnOff.h"

using namespace std;

class DeviceBleSwitchTouchRgb2 : public DeviceBle
{
private:
	ElementButton *elementButton[2];
	ElementRgb *elementRgb[2];
	ElementOnOff * elementOnOff[2];

public:
	DeviceBleSwitchTouchRgb2(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
