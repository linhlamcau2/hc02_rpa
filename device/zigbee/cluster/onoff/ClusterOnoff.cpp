#include "ClusterOnoff.h"
#include "Log.h"

ClusterOnoff::ClusterOnoff(Device *device, uint8_t endpoint) : Cluster(CLUSTER_ONOFF, device, endpoint)
{
	attributeOnoff = new AttributeOnoff(this);
	attributes.push_back(attributeOnoff);
}
