#include "DeviceZigbeeLumiSensorTempHum.h"

DeviceZigbeeLumiSensorTempHum::DeviceZigbeeLumiSensorTempHum(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_LUMI_SENSOR_TEMP_HUM)
{
	clusterTemperature = new ClusterTemperature(this, 1, KEY_ATTRIBUTE_TEMP);
	clusters.push_back(clusterTemperature);
	clusterHumidity = new ClusterHumidity(this, 1, KEY_ATTRIBUTE_HUMIDITY);
	clusters.push_back(clusterHumidity);
}
