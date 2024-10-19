#pragma once

#include "DeviceBle.h"
#include "module/ModuleSmoke.h"
#include "module/ModuleBatteryLevel.h"

using namespace std;

class DeviceBleSmokeSensor : public DeviceBle
{
private:
	ModuleSmoke *moduleSmoke;

public:
	DeviceBleSmokeSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint16_t version);
};
