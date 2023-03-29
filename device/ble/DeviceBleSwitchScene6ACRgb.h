#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "element/ElementRgb.h"

using namespace std;

class DeviceBleSwitchScene6ACRgb : public DeviceBle
{
private:
    ModuleButton *moduleButton[6];
    ElementRgb *elementRgb[6];

public:
    DeviceBleSwitchScene6ACRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
