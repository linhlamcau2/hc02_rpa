#pragma once

#include "DeviceBle.h"
#include "module/ModuleTempHumSoil.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBleTempHumSoilAgriculturalSensor : public DeviceBle
{
private:
	ModuleTempHumSoil *moduleTempHumSoil;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBleTempHumSoilAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
