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
#ifndef CONFIG_USE_MESSAGE_FORMAT_V2
    ModuleRgb *moduleRgb;
    ModuleButton *moduleButton[6];
    int button;
    string idButton[6];
#endif

public:
    DeviceBleSwitchScene6ACRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint8_t button, uint16_t version);
#ifndef CONFIG_USE_MESSAGE_FORMAT_V2
    int GetButton();
#endif
    void InputData(uint8_t *data, int len, uint32_t addr);
};
