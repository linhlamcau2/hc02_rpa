#include "DeviceBleRoolDoor.h"
#include "Log.h"

DeviceBleRoolDoor::DeviceBleRoolDoor(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite)
    : DeviceBle(id, name, mac, data, addr, type, version, isFavorite)
{
    moduleControlClose = new ModuleControlClose(this, addr);
    modules.push_back(moduleControlClose);
    moduleControlOpen = new ModuleControlOpen(this, addr);
    modules.push_back(moduleControlOpen);
    moduleControlPause = new ModuleControlPause(this, addr);
    modules.push_back(moduleControlPause);
    moduleControlPercent = new ModuleControlPercent(this, addr);
    modules.push_back(moduleControlPercent);
    moduleTimeAction = new ModuleTimeActionPir(this, addr);
    modules.push_back(moduleTimeAction);

    powerSource = POWER_AC;
}