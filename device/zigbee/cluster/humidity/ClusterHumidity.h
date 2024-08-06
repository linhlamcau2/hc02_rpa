#pragma once
#include "cluster/Cluster.h"
#include "attribute/AttributeHumidity.h"

#define ATTRIBUTE_HUMIDITY 0x0000

using namespace std;

class ClusterHumidity : public Cluster
{
private:
	AttributeHumidity *attributeHumidity;

protected:
public:
	ClusterHumidity(Device *device, uint8_t endpoint, string humidityKey);
};
