#pragma once

#include "Device.h"

using namespace std;

class DeviceZigbee : public Device
{
protected:
	Json::Value values;

public:
	DeviceZigbee(string id, string name, string mac, uint32_t addr, uint32_t type);
};
