#pragma once

#include "DeviceBle.h"

using namespace std;

class DeviceBleEcTempHumSoilAgriculturalSensor : public DeviceBle
{
private:
public:
	DeviceBleEcTempHumSoilAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
