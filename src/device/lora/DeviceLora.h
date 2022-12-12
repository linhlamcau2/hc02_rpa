#pragma once

#include "Device.h"

using namespace std;

class DeviceLora : public Device
{
protected:
public:
	DeviceLora(string id, string name, string mac, uint32_t addr, uint32_t type);
};
