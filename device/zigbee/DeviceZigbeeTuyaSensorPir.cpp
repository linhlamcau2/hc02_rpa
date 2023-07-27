#include "DeviceZigbeeTuyaSensorPir.h"

DeviceZigbeeTuyaSensorPir::DeviceZigbeeTuyaSensorPir(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_TUYA_SENSOR_PIR_RH3040)
{
}
