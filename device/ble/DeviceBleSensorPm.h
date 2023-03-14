#pragma once

#include "DeviceBle.h"
#include "module/ModulePmSensor.h"
#include "module/ModulePinLevel.h"

using namespace std;

class DeviceBleSensorPm : public DeviceBle
{
private:
	ModulePmSensor *modulePmSensor;

public:
	DeviceBleSensorPm(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);
};
