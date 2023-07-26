#pragma once

#include "DeviceZigbee.h"
#include "cluster/basic/ClusterBasic.h"

using namespace std;

class DeviceZigbeeLumiSensorWleakAQ1 : public DeviceZigbee
{
private:
	ClusterBasic *clusterBasic;

public:
	DeviceZigbeeLumiSensorWleakAQ1(string id, string name, string mac, uint32_t addr);
};
