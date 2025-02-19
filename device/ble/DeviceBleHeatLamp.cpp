
#include "DeviceBleHeatLamp.h"

DeviceBleHeatLamp::DeviceBleHeatLamp(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    moduleControlHeatLamp = new ModuleControlHeatLamp(this, addr);
    modules.push_back(moduleControlHeatLamp);
    modulePeriodHeatLamp = new ModulePeriodHeatLamp(this, addr);
    modules.push_back(modulePeriodHeatLamp);
    moduleStatusLoadHeatLamp = new ModuleStatusLoadHeatLamp(this, addr);
    modules.push_back(moduleStatusLoadHeatLamp);
    moduleTimeOffHeatLamp = new ModuleTimeOffHeatLamp(this, addr);
    modules.push_back(moduleTimeOffHeatLamp);
    moduleTimeOffFanHeatLamp = new ModuleTimeOffFanHeatLamp(this, addr);
    modules.push_back(moduleTimeOffFanHeatLamp);
    moduleTimeDryHeatLamp = new ModuleTimeDryHeatLamp(this, addr);
    modules.push_back(moduleTimeDryHeatLamp);
    powerSource = POWER_AC;
}
