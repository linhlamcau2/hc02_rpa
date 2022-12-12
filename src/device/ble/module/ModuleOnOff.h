#pragma once
#include "Module.h"

using namespace std;

class ModuleOnOff : public Module
{
protected:
	uint8_t onoff;

public:
	ModuleOnOff(Device *device);

	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
	bool CheckData(Json::Value dataValue);
	void CheckTrigger();
	void BuildTelemetryValue(Json::Value &jsonValue);
};
