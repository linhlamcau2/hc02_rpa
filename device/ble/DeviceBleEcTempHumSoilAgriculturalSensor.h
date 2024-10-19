#pragma once

#include "DeviceBle.h"
#include "module/ModuleEcSoil.h"
#include "module/ModuleTempHumSoil.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBleEcTempHumSoilAgriculturalSensor : public DeviceBle
{
private:
	ModuleEcSoil *moduleEcSoil;
	ModuleTempHumSoil *moduleTempHumSoil;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBleEcTempHumSoilAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
