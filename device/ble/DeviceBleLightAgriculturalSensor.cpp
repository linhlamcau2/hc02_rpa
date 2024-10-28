#include "DeviceBleLightAgriculturalSensor.h"

DeviceBleLightAgriculturalSensor::DeviceBleLightAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    moduleLightSensor = new ModuleLightSensor(this, addr);
    modules.push_back(moduleLightSensor);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);
    powerSource = POWER_BATTERY;
}
