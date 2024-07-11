#include "DeviceBleWifiCurtain.h"
#include "module/ModuleCalibAuto.h"
#include "module/ModuleCurtain.h"
#include "module/ModuleModeWifi.h"

#include "Log.h"

DeviceBleWifiCurtain::DeviceBleWifiCurtain(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    ModuleCalibAuto *moduleCalibAuto = new ModuleCalibAuto(this, addr);
    ModuleModeWifi *moduleModeWifi = new ModuleModeWifi(this, addr);
    ModuleCurtain *moduleCurtain = new ModuleCurtain(this, addr);
    modules.push_back(moduleModeWifi);
    modules.push_back(moduleCurtain);
    modules.push_back(moduleCalibAuto);
    powerSource = POWER_AC;
}
