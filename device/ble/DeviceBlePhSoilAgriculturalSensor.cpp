#include "DeviceBlePhSoilAgriculturalSensor.h"

DeviceBlePhSoilAgriculturalSensor::DeviceBlePhSoilAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    modulePhSoil = new ModulePhSoil(this, addr);
    modules.push_back(modulePhSoil);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);

    powerSource = POWER_BATTERY;
}
