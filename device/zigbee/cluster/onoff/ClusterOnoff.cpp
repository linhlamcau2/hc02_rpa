#include "ClusterOnoff.h"
#include "Log.h"

ClusterOnoff::ClusterOnoff(Device *device, uint8_t endpoint, string onoffKey) : Cluster(CLUSTER_ONOFF, device, endpoint)
{
	attributeOnoff = new AttributeOnoff(this, onoffKey);
	attributes.push_back(attributeOnoff);
}
