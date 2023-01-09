#pragma once

#include "DeviceBle.h"
#include "element/ElementOnOff.h"
#include "element/ElementResetNode.h"
#include "element/ElementCct.h"
#include "element/ElementDim.h"

using namespace std;

class DeviceBleDownLightCobTrangTri : public DeviceBle
{
private:
	ElementOnOff *elementOnOff;
	ElementCct *elementCct;
	ElementDim *elementDim;

public:
	DeviceBleDownLightCobTrangTri(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool &rs);
	bool Do(int id, int value);
	bool Do(Json::Value &dataValue);
};
