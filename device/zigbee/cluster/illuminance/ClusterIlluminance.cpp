#include "ClusterIlluminance.h"
#include "Log.h"

ClusterIlluminance::ClusterIlluminance(Device *device, uint8_t endpoint, string illuminanceKey) : Cluster(ZCL_CLUSTER_MS_ILLUMINANCE_MEASUREMENT, device, endpoint)
{
	attributeIlluminance = new AttributeIlluminance(this, illuminanceKey);
	attributes.push_back(attributeIlluminance);
}
