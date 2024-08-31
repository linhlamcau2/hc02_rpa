#include "DeviceBlePhSoilAgriculturalSensor.h"
#include "module/ModulePhSoil.h"
#include "module/ModuleTimeRspSensor.h"

DeviceBlePhSoilAgriculturalSensor::DeviceBlePhSoilAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    ModulePhSoil *modulePhSoil;
    ModuleTimeRspSensor *moduleTimeRspSensor;

    modulePhSoil = new ModulePhSoil(this, addr);
    modules.push_back(modulePhSoil);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);

    powerSource = POWER_BATTERY;
}
