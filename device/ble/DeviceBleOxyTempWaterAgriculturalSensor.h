#pragma once

#include "DeviceBle.h"

using namespace std;

class DeviceBleOxyTempWaterAgriculturalSensor : public DeviceBle
{
private:
public:
	DeviceBleOxyTempWaterAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
