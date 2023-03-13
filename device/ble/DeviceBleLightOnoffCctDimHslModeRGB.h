#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDim.h"
#include "module/ModuleHsl.h"
#include "module/ModuleModeRgb.h"
#include "element/ElementCct.h"

using namespace std;

class DeviceBleLightOnoffCctDimHslModeRGB : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleDim *moduleDim;
	ModuleModeRgb *moduleModeRgb;
	ModuleHsl *moduleHsl;
	ElementCct *elementCct;

public:
	DeviceBleLightOnoffCctDimHslModeRGB(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version);

	bool CheckAddr(uint32_t addr);
	int BuildTelemetryValue(Json::Value &pushDataValue);
	int BuildTelemetryValueV2(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool &rs);
	bool Do(Json::Value &dataValue);
	bool DoV2(Json::Value &dataValue);
	bool AddGroup(uint16_t idGroup, uint16_t epId);
};
