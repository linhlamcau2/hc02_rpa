#include "DeviceZigbee.h"

DeviceZigbee::DeviceZigbee(string id, string name, string mac, uint32_t addr, uint32_t type) : Device(id, name, mac, "", addr, type, 0)
{
	protocol = ZIGBEE_DEVICE;
}
