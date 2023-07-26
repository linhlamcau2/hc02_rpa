#include "DeviceZigbeeLumiSensorMagnet.h"

DeviceZigbeeLumiSensorMagnet::DeviceZigbeeLumiSensorMagnet(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_LUMI_SENSOR_MAGNET)
{
	clusterBasic = new ClusterBasic(this, "model");
	clusters.push_back(clusterBasic);
	clusterOnoff = new ClusterOnoff(this, 1, KEY_ATTRIBUTE_DOOR);
	clusters.push_back(clusterOnoff);
}
