#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementRgb.h"
#include "element/ElementOnOff.h"

using namespace std;

class DeviceBleSwitchTouchRgb1 : public DeviceBle
{
private:
	ElementButton *elementButton;
	ElementRgb *elementRgb;
	ElementOnOff *elementOnOff;

public:
	DeviceBleSwitchTouchRgb1(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
