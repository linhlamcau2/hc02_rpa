#pragma once

#include "DeviceBle.h"
#include "element/ElementOnOff.h"
#include "element/ElementResetNode.h"
#include "element/ElementHsl.h"

using namespace std;

class DeviceBleLedDayRgb : public DeviceBle
{
private:
	ElementOnOff *elementOnOff;
    ElementHsl * elementHsl;
public:
	DeviceBleLedDayRgb(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool& rs);
	bool Do(int id, int value);
    bool Do(Json::Value & dataValue);
};
