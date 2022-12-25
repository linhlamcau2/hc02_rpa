#pragma once

#include "DeviceBle.h"
#include "element/ElementOnOff.h"

using namespace std;

class DeviceBleDownLightSmt : public DeviceBle
{
private:
	ElementOnOff *elementOnOff;

public:
	DeviceBleDownLightSmt(string id, string name, string mac, uint32_t addr);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool& rs);
	bool Do(Json::Value &dataValue);
	bool Do(int id, int value);
};
