#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModulePinLevel.h"

using namespace std;

class DeviceBleDCSceneContact : public DeviceBle
{
private:
	ModuleButton *moduleButton[6];
	ModulePinLevel *modulePinLevel;

public:
	DeviceBleDCSceneContact(string id, string name, string mac, uint32_t addr);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InitAttribute(int attributeId, double value);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool Do(Json::Value &dataValue);
	bool Do(int id, int value);
};
