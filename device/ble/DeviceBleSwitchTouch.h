#pragma once

#include "DeviceBle.h"
#include "module/ModuleRelaySwitch.h"

using namespace std;

class DeviceBleSwitchTouch : public DeviceBle
{
private:
	ModuleRelaySwitch * moduleRelaySwitch;
public:
	DeviceBleSwitchTouch(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version, uint8_t numRelay = 1);
};
