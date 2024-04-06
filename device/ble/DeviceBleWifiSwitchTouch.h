#pragma once

#include "DeviceBle.h"

using namespace std;

class DeviceBleWifiSwitchTouch : public DeviceBle
{
private:
public:
	DeviceBleWifiSwitchTouch(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement);
};
