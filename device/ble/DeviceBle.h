#pragma once

#include "Device.h"

using namespace std;

class DeviceBle : public Device
{
private:
	string deviceKey;

public:
	DeviceBle(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
	string GetDeviceKey(string data);
	string GetDeviceKey();
};
