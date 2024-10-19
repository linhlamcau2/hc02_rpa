#include "DeviceBleEcTempHumSoilAgriculturalSensor.h"

DeviceBleEcTempHumSoilAgriculturalSensor::DeviceBleEcTempHumSoilAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    moduleEcSoil = new ModuleEcSoil(this, addr);
    modules.push_back(moduleEcSoil);
    moduleTempHumSoil = new ModuleTempHumSoil(this, addr);
    modules.push_back(moduleTempHumSoil);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);

    powerSource = POWER_BATTERY;
}
