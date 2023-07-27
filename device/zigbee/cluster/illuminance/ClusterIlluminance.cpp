#include "ClusterIlluminance.h"
#include "Log.h"

ClusterIlluminance::ClusterIlluminance(Device *device, uint8_t endpoint, string illuminanceKey) : Cluster(CLUSTER_ILLUMINANCE, device, endpoint)
{
	attributeIlluminance = new AttributeIlluminance(this, illuminanceKey);
	attributes.push_back(attributeIlluminance);
}
