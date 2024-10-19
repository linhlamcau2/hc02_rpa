#include "DeviceBleOxyTempWaterAgriculturalSensor.h"

DeviceBleOxyTempWaterAgriculturalSensor::DeviceBleOxyTempWaterAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    moduleOxyWater = new ModuleOxyWater(this, addr);
    modules.push_back(moduleOxyWater);
    moduleTempWater = new ModuleTempWater(this, addr);
    modules.push_back(moduleTempWater);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);

    powerSource = POWER_BATTERY;
}
