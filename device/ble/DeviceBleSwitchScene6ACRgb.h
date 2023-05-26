#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "element/ElementRgb.h"
#include "module/ModuleRgb.h"
#include "module/ModuleButton.h"

using namespace std;

class DeviceBleSwitchScene6ACRgb : public DeviceBle
{
private:
#ifndef CONFIG_USE_OLD_APP
    ElementRgb *elementRgb[6];
#endif

#ifdef CONFIG_USE_OLD_APP
    ModuleRgb *moduleRgb;
    ModuleButton *moduleButton[6];
#endif

public:
    DeviceBleSwitchScene6ACRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint8_t button, uint16_t version);
};
