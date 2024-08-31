#pragma once

#include "DeviceBle.h"

using namespace std;

class DeviceBleTempHumAirAgriculturalSensor : public DeviceBle
{
private:

public:
	DeviceBleTempHumAirAgriculturalSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
