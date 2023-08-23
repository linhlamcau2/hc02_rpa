#include "DeviceBleCurtain.h"
#include "Log.h"

DeviceBleCurtain::DeviceBleCurtain(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    moduleCurtain = new ModuleCurtain(this, addr);
    modules.push_back(moduleCurtain);
    moduleRgb = new ModuleRgb(this, addr);
    modules.push_back(moduleRgb);
    // moduleSelectMotor = new ModuleSelectMotor(this, addr);
    // modules.push_back(moduleSelectMotor);
    powerSource = POWER_AC;
}
