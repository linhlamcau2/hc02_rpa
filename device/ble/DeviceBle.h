#pragma once

#include "Device.h"

using namespace std;

class DeviceBle : public Device
{
protected:
	Json::Value values;

public:
	DeviceBle(string id, string name, string mac, uint32_t addr, uint32_t type);
};
