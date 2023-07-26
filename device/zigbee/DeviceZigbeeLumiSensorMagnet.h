#pragma once

#include "DeviceZigbee.h"
#include "cluster/onoff/ClusterOnoff.h"

using namespace std;

class DeviceZigbeeLumiSensorMagnet : public DeviceZigbee
{
private:
	ClusterOnoff *clusterOnoff;

public:
	DeviceZigbeeLumiSensorMagnet(string id, string name, string mac, uint32_t addr);
};
