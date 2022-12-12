#pragma once

#include "DeviceLora.h"

using namespace std;

typedef struct
{
	uint16_t co2;
	uint16_t tovc;
	uint8_t pin;
} co2_sensor_st;

class DeviceLoraCo2Sensor : public DeviceLora
{
private:
	uint16_t co2;
	uint16_t tovc;
	uint8_t pin;

public:
	DeviceLoraCo2Sensor(string id, string name, string mac, uint32_t addr);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool& rs);
	bool Do(Json::Value &dataValue);
};
