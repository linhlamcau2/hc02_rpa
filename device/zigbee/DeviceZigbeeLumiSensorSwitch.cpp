#include "DeviceZigbeeLumiSensorSwitch.h"

DeviceZigbeeLumiSensorSwitch::DeviceZigbeeLumiSensorSwitch(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_LUMI_SENSOR_SWITCH)
{
	clusterBasic = new ClusterBasic(this, "model");
	clusters.push_back(clusterBasic);
	clusterOnoff = new ClusterOnoff(this, 1, KEY_ATTRIBUTE_ONOFF);
	clusters.push_back(clusterOnoff);
}
