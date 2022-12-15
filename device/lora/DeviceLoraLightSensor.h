#pragma once

#include "DeviceLora.h"

using namespace std;

typedef struct
{
	uint32_t lux;
	uint8_t pin;
} light_sensor_st;

class DeviceLoraLightSensor : public DeviceLora
{
private:
	uint32_t lux;
	uint8_t pin;

public:
	DeviceLoraLightSensor(string id, string name, string mac, uint32_t addr);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool& rs);
	bool Do(Json::Value &dataValue);
};
