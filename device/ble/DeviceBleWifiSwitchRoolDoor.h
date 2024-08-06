#pragma once

#include "DeviceBle.h"

using namespace std;

class DeviceBleWifiSwitchRoolDoor : public DeviceBle
{
private:
public:
	DeviceBleWifiSwitchRoolDoor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};
