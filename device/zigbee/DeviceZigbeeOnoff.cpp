#include "DeviceZigbeeOnoff.h"
#include "Log.h"
#include "ZigbeeProtocol.h"

DeviceZigbeeOnoff::DeviceZigbeeOnoff(string id, string name, string mac, Json::Value &dataJson, uint16_t addr)
		: DeviceZigbee(id, name, mac, dataJson, addr, ZIGBEE_LUMI_PLUG)
{
	clusterOnoff = new ClusterOnoff(this, 1, KEY_ATTRIBUTE_ONOFF);
	clusters.push_back(clusterOnoff);
}
