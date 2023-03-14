#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"

using namespace std;

class DeviceBleSwitchTouchRgb1 : public DeviceBle
{
private:
	ElementButton *elementButton;
	ElementRgb *elementRgb;

public:
	DeviceBleSwitchTouchRgb1(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);
};
