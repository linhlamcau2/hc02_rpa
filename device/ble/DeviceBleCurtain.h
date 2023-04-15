#pragma once

#include "DeviceBle.h"
#include "module/ModuleControlClose.h"
#include "module/ModuleControlOpen.h"
#include "module/ModuleControlPause.h"
#include "module/ModuleControlPercent.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleCurtain : public DeviceBle
{
private:
    ModuleControlClose *moduleControlClose;
    ModuleControlOpen *moduleControlOpen;
    ModuleControlPause *moduleControlPause;
    ModuleControlPercent *moduleControlPercent;
    ModuleRgb *moduleRgb;

public:
    DeviceBleCurtain(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
