#pragma once
#include "Cluster.h"
#include "attribute/AttributeOnoff.h"

#define ATTRIBUTE_ONOFF 0x0000

using namespace std;

class ClusterOnoff : public Cluster
{
private:
	AttributeOnoff *attributeOnoff;

protected:
public:
	ClusterOnoff(Device *device, uint8_t endpoint);

	void InitAttribute(int attributeId, double value);
	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
	bool CheckData(Json::Value &dataValue, bool &rs);
	void BuildTelemetryValue(Json::Value &jsonValue);
};
