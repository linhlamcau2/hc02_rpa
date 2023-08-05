#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDimonDimoff.h"

using namespace std;

class DeviceBleSwitchElectrical : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleDimonDimoff *moduleDimonDimoff;

public:
	DeviceBleSwitchElectrical(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version, uint8_t countElement);
};
