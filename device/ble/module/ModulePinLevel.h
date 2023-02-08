#pragma once
#include "Module.h"

using namespace std;

class ModulePinLevel : public Module
{
protected:
	uint16_t pin;

public:
	ModulePinLevel(Device *device);

	bool InputData(uint8_t *data, int len, Json::Value &jsonValue);
	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
	bool CheckData(Json::Value dataValue);
	void CheckTrigger();
	void BuildTelemetryValue(Json::Value &jsonValue);
};
