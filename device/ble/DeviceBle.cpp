#include "DeviceBle.h"

DeviceBle::DeviceBle(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version) : Device(id, name, mac, device_id, addr, type, version)
{
	protocol = BLE_DEVICE;
}

int DeviceBle::AddDevcieSmartHomeToRoom(Room *room)
{
	return 0;
}
