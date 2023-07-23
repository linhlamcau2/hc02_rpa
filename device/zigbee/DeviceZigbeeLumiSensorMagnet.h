#pragma once

#include "DeviceZigbee.h"

using namespace std;

class DeviceZigbeeLumiSensorMagnet : public DeviceZigbee
{
private:
public:
	DeviceZigbeeLumiSensorMagnet(string id, string name, string mac, uint32_t addr);
};
