#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"

using namespace std;

class DeviceBleSwitchOnoff : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;

public:
	DeviceBleSwitchOnoff(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite);
};
