#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModuleDimonDimoff.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleSwitchTouchRgb : public DeviceBle
{
private:
	ModuleButton *moduleButton;
	ModuleDimonDimoff *moduleDimonDimoff;
	ModuleRgb *moduleRgb;

public:
	DeviceBleSwitchTouchRgb(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version, uint8_t element = 0);
};
