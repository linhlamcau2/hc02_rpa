#include "DeviceZigbeeLumiSensorMagnet.h"

DeviceZigbeeLumiSensorMagnet::DeviceZigbeeLumiSensorMagnet(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_LUMI_SENSOR_MAGNET)
{
}
