#pragma once

#include "DeviceBle.h"
#include "module/ModulePhWater.h"
#include "module/ModuleTempWater.h"

using namespace std;

class DeviceBlePhTempWaterAgriculturalSensor : public DeviceBle
{
private:

public:
	DeviceBlePhTempWaterAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
