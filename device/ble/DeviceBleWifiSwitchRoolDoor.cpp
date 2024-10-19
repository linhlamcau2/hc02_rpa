#include "DeviceBleWifiSwitchRoolDoor.h"

DeviceBleWifiSwitchRoolDoor::DeviceBleWifiSwitchRoolDoor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    moduleCalibAuto = new ModuleCalibAuto(this, addr);
    moduleCountDownSwitch = new ModuleCountDownSwitch(this, addr);
    moduleLock = new ModuleLock(this, addr);
    moduleModeWifi = new ModuleModeWifi(this, addr);
    moduleCurtain = new ModuleCurtain(this, addr);
    modules.push_back(moduleModeWifi);
    modules.push_back(moduleCountDownSwitch);
    modules.push_back(moduleLock);
    modules.push_back(moduleCurtain);
    modules.push_back(moduleCalibAuto);
    powerSource = POWER_AC;
}
