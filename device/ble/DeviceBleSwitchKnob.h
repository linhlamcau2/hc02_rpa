#pragma once

#include "DeviceBle.h"

using namespace std;

class DeviceBleSwitchKnob : public DeviceBle
{
private:
public:
	DeviceBleSwitchKnob(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement);
};
