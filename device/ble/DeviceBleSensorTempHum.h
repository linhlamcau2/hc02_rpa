#pragma once

#include "DeviceBle.h"
#include "module/ModuleTempHum.h"
#include "module/ModulePinLevel.h"

using namespace std;

class DeviceBleSensorTempHum : public DeviceBle
{
private:
	ModuleTempHum *moduleTempHum;
	ModulePinLevel *modulePinLevel;

public:
	DeviceBleSensorTempHum(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);
};
