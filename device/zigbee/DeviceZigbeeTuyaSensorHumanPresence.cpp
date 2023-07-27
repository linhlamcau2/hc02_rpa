#include "DeviceZigbeeTuyaSensorHumanPresence.h"

DeviceZigbeeTuyaSensorHumanPresence::DeviceZigbeeTuyaSensorHumanPresence(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_TUYA_SENSOR_HUMAN_PRESENCE_TS0225)
{
	clusterIlluminance = new ClusterIlluminance(this, 1, KEY_ATTRIBUTE_PIR);
	clusters.push_back(clusterIlluminance);
}
