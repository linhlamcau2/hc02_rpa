#include "DeviceBleSwitchScene6ACRgb.h"
#include "Log.h"

DeviceBleSwitchScene6ACRgb::DeviceBleSwitchScene6ACRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint8_t button, uint16_t version)
    : DeviceBle(id, name, mac, data, addr, type, version)
{
#ifndef CONFIG_USE_OLD_APP
    for (int i = 0; i < 6; i++)
    {
        elementRgb[i] = new ElementRgb(this, addr + i);
        elements.push_back(elementRgb[i]);
    }
    countElement = 6;
#endif

    // powerSource = POWER_AC;
#ifdef CONFIG_USE_OLD_APP
    moduleRgb = new ModuleRgb(this, addr, button);
    modules.push_back(moduleRgb);
    for (int i = 0; i < 6; i++)
    {
        moduleButton[i] = new ModuleButton(this, addr, i);
        modules.push_back(moduleButton[i]);
    }
#endif
}
