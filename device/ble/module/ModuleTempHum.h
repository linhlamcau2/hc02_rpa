#pragma once
#include "Module.h"

using namespace std;

class ModuleTempHum : public Module
{
protected:
	float temp;
	float hum;

public:
	ModuleTempHum(Device *device);

	float GetTemp();
	void SetTemp(float temp);
	float GetHum();
	void SetHum(float hum);

	bool InputData(uint8_t *data, int len, Json::Value &jsonValue);
	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
	bool CheckData(Json::Value dataValue);
	void CheckTrigger();
	void BuildTelemetryValue(Json::Value &jsonValue);
};
