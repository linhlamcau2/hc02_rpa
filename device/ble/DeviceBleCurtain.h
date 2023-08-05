#pragma once

#include "DeviceBle.h"
#include "module/ModuleCurtain.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleCurtain : public DeviceBle
{
private:
	ModuleCurtain *moduleCurtain;
	ModuleRgb *moduleRgb;

public:
	DeviceBleCurtain(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version);
};
