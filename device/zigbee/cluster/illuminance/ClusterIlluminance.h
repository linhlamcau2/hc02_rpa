#pragma once
#include "zigbee/cluster/Cluster.h"
#include "attribute/AttributeIlluminance.h"

#define ATTRIBUTE_ILLUMINANCE 0x0000

using namespace std;

class ClusterIlluminance : public Cluster
{
private:
	AttributeIlluminance *attributeIlluminance;

protected:
public:
	ClusterIlluminance(Device *device, uint8_t endpoint, string illuminanceKey);
};
