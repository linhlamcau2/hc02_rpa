#pragma once

#include "DeviceBle.h"
#include "module/ModuleLightSensor.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBleLightAgriculturalSensor : public DeviceBle
{
private:
	ModuleLightSensor *moduleLightSensor;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBleLightAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
