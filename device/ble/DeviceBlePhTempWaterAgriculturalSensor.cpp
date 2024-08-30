#include "DeviceBlePhTempWaterAgriculturalSensor.h"

DeviceBlePhTempWaterAgriculturalSensor::DeviceBlePhTempWaterAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    ModulePhWater * modulePhWater;
    ModuleTempWater * moduleTempWater;

    modulePhWater = new ModulePhWater(this, addr);
    modules.push_back(modulePhWater);
    moduleTempWater = new ModuleTempWater(this, addr);
    modules.push_back(moduleTempWater);
    
    powerSource = POWER_BATTERY;
}
