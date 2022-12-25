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

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InitAttribute(int attributeId, double value);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool CheckData(Json::Value &dataValue, bool &rs);
	bool Do(Json::Value &dataValue);
};
