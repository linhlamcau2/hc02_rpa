#pragma once

#include "DeviceZigbee.h"
#include "cluster/basic/ClusterBasic.h"
#include "cluster/temperature/ClusterTemperature.h"
#include "cluster/humidity/ClusterHumidity.h"

using namespace std;

class DeviceZigbeeLumiSensorTempHum : public DeviceZigbee
{
private:
	ClusterBasic *clusterBasic;
	ClusterTemperature *clusterTemperature;
	ClusterHumidity *clusterHumidity;

public:
	DeviceZigbeeLumiSensorTempHum(string id, string name, string mac, Json::Value &dataJson, uint16_t addr);
};
