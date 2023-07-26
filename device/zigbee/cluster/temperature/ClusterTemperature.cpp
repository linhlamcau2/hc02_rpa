#include "ClusterTemperature.h"
#include "Log.h"

ClusterTemperature::ClusterTemperature(Device *device, uint8_t endpoint, string temperatureKey) : Cluster(CLUSTER_TEMPERATURE, device, endpoint)
{
	attributeTemperature = new AttributeTemperature(this, temperatureKey);
	attributes.push_back(attributeTemperature);
}
