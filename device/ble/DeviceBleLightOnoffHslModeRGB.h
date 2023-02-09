#pragma once

#include "DeviceBle.h"
#include "ModuleOnOff.h"
#include "ModuleHsl.h"
#include "ModuleModeRgb.h"

using namespace std;

class DeviceBleLightOnoffHslModeRGB : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleModeRgb *moduleModeRgb;
	ModuleHsl *moduleHsl;

public:
	DeviceBleLightOnoffHslModeRGB(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version);

	bool CheckAddr(uint32_t addr);
	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool &rs);
	bool Do(Json::Value &dataValue);
	bool AddGroup(uint16_t idGroup, uint16_t epId);
};
