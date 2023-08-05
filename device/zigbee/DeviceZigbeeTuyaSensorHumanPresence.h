#pragma once

#include "DeviceZigbee.h"
#include "cluster/illuminance/ClusterIlluminance.h"

using namespace std;

class DeviceZigbeeTuyaSensorHumanPresence : public DeviceZigbee
{
private:
	ClusterIlluminance *clusterIlluminance;

public:
	DeviceZigbeeTuyaSensorHumanPresence(string id, string name, string mac, Json::Value &dataJson, uint32_t addr);
};
