#pragma once

#include "DeviceBle.h"
#include "element/ElementOnOff.h"

using namespace std;

class DeviceBleSwitch4 : public DeviceBle
{
private:
	ElementOnOff *elementOnOff[4];

public:
	DeviceBleSwitch4(string id, string name, string mac, uint32_t addr);

	bool CheckAddr(uint32_t addr);

	int BuildTelemetryValue(Json::Value &pushDataValue);

#ifdef CONFIG_FPT_SERVER
	void InitAttribute(int attributeId, double value);
#endif
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool &rs);
	bool Do(Json::Value &dataValue);
	bool Do(int id, int value);
};
