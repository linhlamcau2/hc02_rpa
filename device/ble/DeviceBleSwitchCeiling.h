#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCallScene.h"

using namespace std;

class DeviceBleSwitchCeiling : public DeviceBle
{
private:
public:
    DeviceBleSwitchCeiling(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version, uint8_t element = 0);
};
