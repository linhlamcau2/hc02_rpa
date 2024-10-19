#include "DeviceBleTempHumAirAgriculturalSensor.h"

DeviceBleTempHumAirAgriculturalSensor::DeviceBleTempHumAirAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    moduleTempHum = new ModuleTempHum(this, addr);
    modules.push_back(moduleTempHum);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);

    powerSource = POWER_BATTERY;
}
