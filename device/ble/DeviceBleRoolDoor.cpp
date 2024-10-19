#include "DeviceBleRoolDoor.h"

DeviceBleRoolDoor::DeviceBleRoolDoor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	moduleCurtain = new ModuleCurtain(this, addr);
	modules.push_back(moduleCurtain);
	moduleTimeAction = new ModuleTimeActionPir(this, addr);
	modules.push_back(moduleTimeAction);
	powerSource = POWER_AC;
}