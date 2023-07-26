#include "DeviceZigbeeLumiSensorTempHum.h"

DeviceZigbeeLumiSensorTempHum::DeviceZigbeeLumiSensorTempHum(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_LUMI_SENSOR_TEMP_HUM)
{
	clusterBasic = new ClusterBasic(this, "model");
	clusters.push_back(clusterBasic);
	clusterTemperature = new ClusterTemperature(this, 1, KEY_ATTRIBUTE_TEMP);
	clusters.push_back(clusterTemperature);
	clusterHumidity = new ClusterHumidity(this, 1, KEY_ATTRIBUTE_HUMIDITY);
	clusters.push_back(clusterHumidity);
}
