#pragma once

#include "DeviceBle.h"
#include "ModuleOnOff.h"
#include "ModuleDim.h"
#include "ElementCct.h"

using namespace std;

class DeviceBleOnoffCctDim : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleDim *moduleDim;
	ElementCct *elementCct;

public:
	DeviceBleOnoffCctDim(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version);

	bool CheckAddr(uint32_t addr);
	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool &rs);
	bool Do(Json::Value &dataValue);
};
