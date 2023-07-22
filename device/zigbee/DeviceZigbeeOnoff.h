#pragma once

#include "DeviceZigbee.h"
#include "cluster/onoff/ClusterOnoff.h"

using namespace std;

class DeviceZigbeeOnoff : public DeviceZigbee
{
private:
	ClusterOnoff *clusterOnoff;

public:
	DeviceZigbeeOnoff(string id, string name, string mac, uint32_t addr);
	
	int Do(Json::Value &dataValue);
};
