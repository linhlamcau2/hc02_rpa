#include "ClusterHumidity.h"
#include "Log.h"

ClusterHumidity::ClusterHumidity(Device *device, uint8_t endpoint, string humidityKey) : Cluster(ZCL_CLUSTER_MS_RELATIVE_HUMIDITY, device, endpoint)
{
	attributeHumidity = new AttributeHumidity(this, humidityKey);
	attributes.push_back(attributeHumidity);
}
