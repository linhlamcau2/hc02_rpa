#include "DeviceBleSwitchScene6ACRgb.h"
#include "Log.h"

DeviceBleSwitchScene6ACRgb::DeviceBleSwitchScene6ACRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, data, addr, type, version)
{
    for (int i = 0; i < 6; i++)
    {
        moduleButton[i] = new ModuleButton(this, addr, i);
        elementRgb[i] = new ElementRgb(this, addr + i);
        modules.push_back(moduleButton[i]);
        elements.push_back(elementRgb[i]);
    }
    countElement = 6;
    powerSource = POWER_AC;
}
