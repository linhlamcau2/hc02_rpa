#pragma once

#include "Device.h"
#include "cluster/Cluster.h"

using namespace std;

class DeviceZigbee : public Device
{
protected:
	vector<Cluster *> clusters;

public:
	DeviceZigbee(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type);

	virtual int BuildTelemetryValue(Json::Value &pushDataValue);

	virtual void InputData(Json::Value &dataValue);
	virtual void InputData(uint8_t *data, int len);
	virtual bool CheckData(Json::Value &dataValue, bool &rs);

	virtual int Do(Json::Value &dataValue);
};
