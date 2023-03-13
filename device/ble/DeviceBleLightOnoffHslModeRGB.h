#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleHsl.h"
#include "module/ModuleModeRgb.h"

using namespace std;

class DeviceBleLightOnoffHslModeRGB : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleModeRgb *moduleModeRgb;
	ModuleHsl *moduleHsl;

public:
	DeviceBleLightOnoffHslModeRGB(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version);
};
