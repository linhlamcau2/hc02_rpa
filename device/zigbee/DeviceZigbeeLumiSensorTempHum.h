#pragma once

#include "DeviceZigbee.h"
#include "cluster/temperature/ClusterTemperature.h"
#include "cluster/humidity/ClusterHumidity.h"

using namespace std;

class DeviceZigbeeLumiSensorTempHum : public DeviceZigbee
{
private:
	ClusterTemperature *clusterTemperature;
	ClusterHumidity *clusterHumidity;

public:
	DeviceZigbeeLumiSensorTempHum(string id, string name, string mac, uint32_t addr);
};
