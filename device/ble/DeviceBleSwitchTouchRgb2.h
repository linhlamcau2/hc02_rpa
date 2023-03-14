#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"

using namespace std;

class DeviceBleSwitchTouchRgb2 : public DeviceBle
{
private:
	ElementButton *elementButton[2];
	ElementRgb *elementRgb[2];

public:
	DeviceBleSwitchTouchRgb2(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);
};
