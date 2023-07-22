#include "DeviceZigbeeOnoff.h"
#include "Log.h"
#include "ZigbeeProtocol.h"

DeviceZigbeeOnoff::DeviceZigbeeOnoff(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_LUMI_PLUG)
{
	clusterOnoff = new ClusterOnoff(this, 1);
	clusters.push_back(clusterOnoff);
}
