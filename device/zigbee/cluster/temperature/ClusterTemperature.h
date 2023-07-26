#pragma once
#include "zigbee/cluster/Cluster.h"
#include "attribute/AttributeTemperature.h"

#define ATTRIBUTE_TEMPERATURE 0x0000

using namespace std;

class ClusterTemperature : public Cluster
{
private:
	AttributeTemperature *attributeTemperature;

protected:
public:
	ClusterTemperature(Device *device, uint8_t endpoint, string temperatureKey);
};
