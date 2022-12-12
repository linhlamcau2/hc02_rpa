#pragma once

#include "DeviceBle.h"
#include "module/ModuleTempHum.h"
#include "module/ModulePinLevel.h"

using namespace std;

class DeviceBleTempHumSensor : public DeviceBle
{
private:
	ModuleTempHum *moduleTempHum;
	ModulePinLevel *modulePinLevel;

public:
	DeviceBleTempHumSensor(string id, string name, string mac, uint32_t addr);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool Do(Json::Value &dataValue);
};
