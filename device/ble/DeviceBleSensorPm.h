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

	int BuildTelemetryValue(Json::Value &pushDataValue);
	int BuildTelemetryValueV2(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
};
