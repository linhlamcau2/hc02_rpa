#pragma once

#include "DeviceBle.h"
#include "module/ModuleCurtain.h"
#include "module/ModuleTimeActionPir.h"

using namespace std;

class DeviceBleRoolDoor : public DeviceBle
{
private:
    ModuleCurtain *moduleCurtain;
    ModuleTimeActionPir *moduleTimeAction;

public:
    DeviceBleRoolDoor(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
