#pragma once

#include "DeviceBle.h"
#include "module/ModulePhWater.h"
#include "module/ModuleTempWater.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBlePhTempWaterAgriculturalSensor : public DeviceBle
{
private:
	ModulePhWater *modulePhWater;
	ModuleTempWater *moduleTempWater;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBlePhTempWaterAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
