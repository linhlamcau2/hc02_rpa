#pragma once

#include "DeviceZigbee.h"
#include "cluster/basic/ClusterBasic.h"
#include "cluster/onoff/ClusterOnoff.h"

using namespace std;

class DeviceZigbeeLumiSensorSwitch : public DeviceZigbee
{
private:
	ClusterBasic *clusterBasic;
	ClusterOnoff *clusterOnoff;

public:
	DeviceZigbeeLumiSensorSwitch(string id, string name, string mac, Json::Value &dataJson, uint32_t addr);
};
