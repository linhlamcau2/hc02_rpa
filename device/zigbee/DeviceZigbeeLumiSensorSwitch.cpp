#include "DeviceZigbeeLumiSensorSwitch.h"

DeviceZigbeeLumiSensorSwitch::DeviceZigbeeLumiSensorSwitch(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_LUMI_SENSOR_SWITCH)
{
	clusterOnoff = new ClusterOnoff(this, 1);
	clusters.push_back(clusterOnoff);
}
