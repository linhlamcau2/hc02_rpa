#pragma once

#include "DeviceBle.h"
#include "module/ModuleSmoke.h"
#include "module/ModulePinLevel.h"

using namespace std;

class DeviceBleSmokeSensor : public DeviceBle
{
private:
	ModuleSmoke *moduleSmoke;

public:
	DeviceBleSmokeSensor(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);
};
