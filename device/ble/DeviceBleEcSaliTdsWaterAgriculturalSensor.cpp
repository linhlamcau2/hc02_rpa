#include "DeviceBleEcSaliTdsWaterAgriculturalSensor.h"

DeviceBleEcSaliTdsWaterAgriculturalSensor::DeviceBleEcSaliTdsWaterAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    moduleEcWater = new ModuleEcWater(this, addr);
    modules.push_back(moduleEcWater);
    moduleTempWater = new ModuleTempWater(this, addr);
    modules.push_back(moduleTempWater);
    moduleEcSaliTds = new ModuleEcSaliTds(this, addr);
    modules.push_back(moduleEcSaliTds);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);

    powerSource = POWER_BATTERY;
}
