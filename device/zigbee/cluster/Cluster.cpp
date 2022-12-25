#include "Cluster.h"

Cluster::Cluster(Device *device, uint8_t endpoint)
{
	this->device = device;
	this->endpoint = endpoint;
}
