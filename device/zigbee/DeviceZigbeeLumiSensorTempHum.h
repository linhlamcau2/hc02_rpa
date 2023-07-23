#pragma once

#include "DeviceZigbee.h"

using namespace std;

class DeviceZigbeeLumiSensorTempHum : public DeviceZigbee
{
private:
public:
	DeviceZigbeeLumiSensorTempHum(string id, string name, string mac, uint32_t addr);
};
