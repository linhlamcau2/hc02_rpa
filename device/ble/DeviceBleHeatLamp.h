#pragma once

#include "DeviceBle.h"
#include "module/ModuleControlHeatLamp.h"
#include "module/ModulePeriodHeatLamp.h"
#include "module/ModuleStatusLoadHeatLamp.h"
#include "module/ModuleTimeOffHeatLamp.h"
#include "module/ModuleTimeOffFanHeatLamp.h"
#include "module/ModuleTimeDryHeatLamp.h"

using namespace std;

class DeviceBleHeatLamp : public DeviceBle
{
private:
    ModuleControlHeatLamp *moduleControlHeatLamp;
    ModulePeriodHeatLamp *modulePeriodHeatLamp;
    ModuleStatusLoadHeatLamp *moduleStatusLoadHeatLamp;
    ModuleTimeOffHeatLamp *moduleTimeOffHeatLamp;
    ModuleTimeOffFanHeatLamp *moduleTimeOffFanHeatLamp;
    ModuleTimeDryHeatLamp *moduleTimeDryHeatLamp;

public:
    DeviceBleHeatLamp(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version);
};
