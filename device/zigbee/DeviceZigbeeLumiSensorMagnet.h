#pragma once

#include "DeviceZigbee.h"
#include "cluster/basic/ClusterBasic.h"
#include "cluster/onoff/ClusterOnoff.h"

using namespace std;

class DeviceZigbeeLumiSensorMagnet : public DeviceZigbee
{
private:
	ClusterBasic *clusterBasic;
	ClusterOnoff *clusterOnoff;

public:
	DeviceZigbeeLumiSensorMagnet(string id, string name, string mac, Json::Value &dataJson, uint16_t addr);
};
