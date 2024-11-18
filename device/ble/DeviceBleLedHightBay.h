#pragma once

#include "DeviceBle.h"
#include "module/ModulePirSensor.h"
#include "module/ModuleTimeActionPir.h"
#include "module/ModuleModeActionPir.h"
#include "module/ModuleSensiPir.h"
#include "module/ModuleDimLevel.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDim.h"

using namespace std;

class DeviceBleLedHightBay : public DeviceBle
{
private:
    ModulePirSensor *modulePirSensor;
    ModuleTimeActionPir *moduleTimeActionPir;
    ModuleModeActionPir *moduleModeActionPir;
    ModuleSensiPir *moduleSensiPir;
    ModuleDimLevel *moduleDimLevel;
    ModuleStatusStartup *moduleStatusStartup;
    ModuleOnOff *moduleOnOff;
    ModuleDim *moduleDim;

public:
    DeviceBleLedHightBay(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version);
};
