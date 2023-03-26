#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"

using namespace std;

class DeviceBleSwitchTouchRgb4 : public DeviceBle
{
private:
	ElementButton *elementButton[4];
	ElementRgb *elementRgb[4];

public:
	DeviceBleSwitchTouchRgb4(string id, string name, string mac, string data, uint32_t addr, uint16_t version);
};
