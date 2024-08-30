#include "DeviceBleLightAgriculturalSensor.h"

DeviceBleLightAgriculturalSensor::DeviceBleLightAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    ModuleLightSensor * moduleLightSensor;
    moduleLightSensor = new ModuleLightSensor(this, addr);
    modules.push_back(moduleLightSensor);
    powerSource = POWER_BATTERY;
}
