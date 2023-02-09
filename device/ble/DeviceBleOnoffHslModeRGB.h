#pragma once

#include "DeviceBle.h"
#include "element/ElementOnOff.h"
#include "element/ElementResetNode.h"
#include "element/ElementCct.h"
#include "element/ElementDim.h"
#include "element/ElementHsl.h"
#include "element/ElementModeRgb.h"

using namespace std;

class DeviceBleOnoffHslModeRGB : public DeviceBle
{
private:
	ElementOnOff *elementOnOff;
	ElementHsl *elementHsl;
	ElementModeRgb *elementModeRgb;

public:
	DeviceBleOnoffHslModeRGB(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version);

	bool CheckAddr(uint32_t addr);
	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool &rs);
	bool Do(Json::Value &dataValue);
	bool AddGroup(uint16_t idGroup, uint16_t epId);
};
