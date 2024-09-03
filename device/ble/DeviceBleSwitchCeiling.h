#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleAllRelay.h"

using namespace std;

class DeviceBleSwitchCeiling : public DeviceBle
{
private:
    ModuleButton *moduleButton;
    ModuleOnOff *moduleOnOff;
    ModuleCountDownSwitch *moduleCountDownSwitch;
    ModuleStatusStartup *moduleStatusStartup;
    ModuleAllRelay *moduleAllRelay;

public:
    DeviceBleSwitchCeiling(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, uint8_t element = 0);
    int BuildTelemetryValue(Json::Value &pushDataValue);
};
