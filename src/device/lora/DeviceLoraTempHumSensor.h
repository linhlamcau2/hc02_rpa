#pragma once

#include "DeviceLora.h"

using namespace std;

typedef struct
{
	uint16_t temp;
	uint16_t hum;
	uint8_t pin;
} temp_hum_sensor_st;

class DeviceLoraTempHumSensor : public DeviceLora
{
private:
	float temp;
	float hum;
	uint8_t pin;

public:
	DeviceLoraTempHumSensor(string id, string name, string mac, uint32_t addr);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool& rs);
	bool Do(Json::Value &dataValue);
};
