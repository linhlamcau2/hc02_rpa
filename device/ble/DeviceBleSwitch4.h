#pragma once

#include "DeviceBle.h"
#include "element/ElementButton.h"
#include "element/ElementResetNode.h"

using namespace std;

class DeviceBleSwitch4 : public DeviceBle
{
private:
	ElementButton *elementButton[4];

public:
	DeviceBleSwitch4(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);

	bool CheckAddr(uint32_t addr);

	int BuildTelemetryValue(Json::Value &pushDataValue);

#ifdef CONFIG_SAVE_ATTRIBUTE
	void InitAttribute(int attributeId, double value);
#endif
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool &rs);
	bool Do(Json::Value &dataValue);
};
