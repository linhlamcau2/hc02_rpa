#include "DeviceBleRoolDoor.h"
#include "Log.h"

DeviceBleRoolDoor::DeviceBleRoolDoor(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, data, addr, type, version)
{
    moduleCurtain = new ModuleCurtain(this, addr);
    modules.push_back(moduleCurtain);
    moduleTimeAction = new ModuleTimeActionPir(this, addr);
    modules.push_back(moduleTimeAction);
    powerSource = POWER_AC;
}