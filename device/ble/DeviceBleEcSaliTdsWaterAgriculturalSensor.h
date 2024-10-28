#pragma once

#include "DeviceBle.h"
#include "module/ModuleEcWater.h"
#include "module/ModuleTempWater.h"
#include "module/ModuleEcSaliTds.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBleEcSaliTdsWaterAgriculturalSensor : public DeviceBle
{
private:
	ModuleEcWater *moduleEcWater;
	ModuleTempWater *moduleTempWater;
	ModuleEcSaliTds *moduleEcSaliTds;
    ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBleEcSaliTdsWaterAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
