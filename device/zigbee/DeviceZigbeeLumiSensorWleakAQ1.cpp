#include "DeviceZigbeeLumiSensorWleakAQ1.h"

DeviceZigbeeLumiSensorWleakAQ1::DeviceZigbeeLumiSensorWleakAQ1(string id, string name, string mac, Json::Value &dataJson, uint16_t addr)
		: DeviceZigbee(id, name, mac, dataJson, addr, ZIGBEE_LUMI_SENSOR_WLEAK_AQ1)
{
	clusterBasic = new ClusterBasic(this);
	clusters.push_back(clusterBasic);
}
