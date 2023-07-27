#include "ClusterOnoff.h"
#include "Log.h"

ClusterOnoff::ClusterOnoff(Device *device, uint8_t endpoint, string onoffKey) : Cluster(ZCL_CLUSTER_GEN_ON_OFF, device, endpoint)
{
	attributeOnoff = new AttributeOnoff(this, onoffKey);
	attributes.push_back(attributeOnoff);
}
