#pragma once

#include "DeviceLora.h"

using namespace std;

typedef struct
{
	uint8_t mois;
	uint8_t temp;
	uint16_t ec;
	uint8_t pin;
} soil_temp_mois_sensor_st;

class DeviceLoraECSensor : public DeviceLora
{
private:
	uint8_t temp;
	uint8_t mois;
	uint16_t ec;
	uint8_t pin;

public:
	DeviceLoraECSensor(string id, string name, string mac, uint32_t addr);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool& rs);
	bool Do(Json::Value &dataValue);
};
