#include "DeviceZigbeeTuyaSensorPir.h"

DeviceZigbeeTuyaSensorPir::DeviceZigbeeTuyaSensorPir(string id, string name, string mac, Json::Value &dataJson, uint32_t addr)
		: DeviceZigbee(id, name, mac, dataJson, addr, ZIGBEE_TUYA_SENSOR_PIR_RH3040)
{
}
