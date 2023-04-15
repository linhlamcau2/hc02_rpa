#include "DeviceBleCurtain.h"
#include "Log.h"

DeviceBleCurtain::DeviceBleCurtain(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, data, addr, type, version)
{
    moduleControlClose = new ModuleControlClose(this, addr);
    modules.push_back(moduleControlClose);
    moduleControlOpen = new ModuleControlOpen(this, addr);
    modules.push_back(moduleControlOpen);
    moduleControlPause = new ModuleControlPause(this, addr);
    modules.push_back(moduleControlPause);
    moduleControlPercent = new ModuleControlPercent(this, addr);
    modules.push_back(moduleControlPercent);
    moduleRgb = new ModuleRgb(this, addr, 0);
    modules.push_back(moduleRgb);

    powerSource = POWER_AC;
}
