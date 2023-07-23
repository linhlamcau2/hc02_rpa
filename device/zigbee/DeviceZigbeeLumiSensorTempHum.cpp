#include "DeviceZigbeeLumiSensorTempHum.h"

DeviceZigbeeLumiSensorTempHum::DeviceZigbeeLumiSensorTempHum(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_LUMI_SENSOR_TEMP_HUM)
{
}
