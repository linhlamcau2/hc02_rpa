#pragma once

#include "DeviceBle.h"
#include "module/ModuleDoorHangOn.h"
#include "module/ModuleDoorStatus.h"
#include "module/ModulePinLevel.h"

using namespace std;

class DeviceBleDoorSensor : public DeviceBle
{
private:
	ModuleDoorHangOn *moduleDoorHangOn;
	ModuleDoorStatus *moduleDoorStatus;
	ModulePinLevel *modulePinLevel;

public:
	DeviceBleDoorSensor(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);
};
