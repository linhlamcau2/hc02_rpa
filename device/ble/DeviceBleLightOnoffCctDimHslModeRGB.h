#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDim.h"
#include "module/ModuleHsl.h"
#include "module/ModuleModeRgb.h"
#include "module/ModuleCallScene.h"
#include "module/ModuleCct.h"

using namespace std;

class DeviceBleLightOnoffCctDimHslModeRGB : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleDim *moduleDim;
	ModuleModeRgb *moduleModeRgb;
	ModuleHsl *moduleHsl;
	ModuleCallScene *moduleCallScene;
	ModuleCct *moduleCct;

public:
	DeviceBleLightOnoffCctDimHslModeRGB(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};
