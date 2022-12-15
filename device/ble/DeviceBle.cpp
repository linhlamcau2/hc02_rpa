#include "DeviceBle.h"

DeviceBle::DeviceBle(string id, string name, string mac, uint32_t addr, uint32_t type) : Device(id, name, mac, addr, type)
{
	protocol = BLE_DEVICE;
}
