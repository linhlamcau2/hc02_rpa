
#include "DeviceBleLedHightBay.h"

DeviceBleLedHightBay::DeviceBleLedHightBay(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    modulePirSensor = new ModulePirSensor(this, addr);
    moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
    moduleModeActionPir = new ModuleModeActionPir(this, addr);
    moduleSensiPir = new ModuleSensiPir(this, addr);
    moduleDimLevel = new ModuleDimLevel(this, addr);
    moduleStatusStartup = new ModuleStatusStartup(this, addr);
    moduleOnOff = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_ONOFF);
    moduleDim = new ModuleDim(this, addr);

    modules.push_back(modulePirSensor);
    modules.push_back(moduleTimeActionPir);
    modules.push_back(moduleModeActionPir);
    modules.push_back(moduleSensiPir);
    modules.push_back(moduleDimLevel);
    modules.push_back(moduleStatusStartup);
    modules.push_back(moduleOnOff);
    modules.push_back(moduleDim);

    powerSource = POWER_AC;
}
