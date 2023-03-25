#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDim.h"

using namespace std;

class DeviceBleAll : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleDim *moduleDim;

public:
	DeviceBleAll(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version);
};
