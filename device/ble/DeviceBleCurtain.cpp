#include "DeviceBleCurtain.h"
#include "Log.h"

DeviceBleCurtain::DeviceBleCurtain(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, data, addr, type, version)
{
    moduleCurtain = new ModuleCurtain(this, addr);
    modules.push_back(moduleCurtain);
    moduleRgb = new ModuleRgb(this, addr);
    modules.push_back(moduleRgb);
    powerSource = POWER_AC;
}
