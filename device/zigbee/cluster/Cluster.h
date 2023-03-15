#pragma once

#include <string>
#include "json.h"

#define PROFILE_ZHA 0x0104

#define CLUSTER_GENERAL_BASIC 0x0000
#define CLUSTER_ONOFF 0x0006

using namespace std;

class Device;
class Cluster
{
protected:
	Device *device;
	uint8_t endpoint;

public:
	Cluster(Device *device, uint8_t endpoint);

	Device *getDevice() { return device; }

	virtual void ParseData(uint8_t *data, int len, Json::Value &jsonValue) {}
	virtual bool CheckData(Json::Value &dataValue, bool &rs) { return false; }
};
