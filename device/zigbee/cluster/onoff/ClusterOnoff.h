#pragma once
#include "cluster/Cluster.h"
#include "attribute/AttributeOnoff.h"

#define ATTRIBUTE_ONOFF 0x0000

using namespace std;

class ClusterOnoff : public Cluster
{
private:
	AttributeOnoff *attributeOnoff;

protected:
public:
	ClusterOnoff(Device *device, uint8_t endpoint, string onoffKey);
};
