#pragma once

#include "DeviceZigbee.h"

using namespace std;

class DeviceZigbeeLumiSensorWleakAQ1 : public DeviceZigbee
{
private:
public:
	DeviceZigbeeLumiSensorWleakAQ1(string id, string name, string mac, uint32_t addr);
};
