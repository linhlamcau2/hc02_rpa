#pragma once

#include "DeviceZigbee.h"
#include "cluster/onoff/ClusterOnoff.h"

using namespace std;

class DeviceZigbeeLumiSensorSwitch : public DeviceZigbee
{
private:
	ClusterOnoff *clusterOnoff;

public:
	DeviceZigbeeLumiSensorSwitch(string id, string name, string mac, uint32_t addr);
};
