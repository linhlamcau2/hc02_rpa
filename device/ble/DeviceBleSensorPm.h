#pragma once

#include "DeviceBle.h"
#include "module/ModulePmSensor.h"
#include "module/ModuleTempHum.h"
#include "module/ModulePinLevel.h"

using namespace std;

class DeviceBleSensorPm : public DeviceBle
{
private:
	ModulePmSensor *modulePmSensor;
	ModuleTempHum *moduleTempHum;

public:
	DeviceBleSensorPm(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite);
};
