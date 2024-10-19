#pragma once

#include "DeviceBle.h"
#include "module/ModuleOxyWater.h"
#include "module/ModuleTempWater.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBleOxyTempWaterAgriculturalSensor : public DeviceBle
{
private:
	ModuleOxyWater *moduleOxyWater;
	ModuleTempWater *moduleTempWater;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBleOxyTempWaterAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
