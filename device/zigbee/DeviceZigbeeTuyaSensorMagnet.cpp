#include "DeviceZigbeeTuyaSensorMagnet.h"

DeviceZigbeeTuyaSensorMagnet::DeviceZigbeeTuyaSensorMagnet(string id, string name, string mac, Json::Value &dataJson, uint16_t addr)
		: DeviceZigbee(id, name, mac, dataJson, addr, ZIGBEE_TUYA_SENSOR_MAGNET_TY0203)
{
}
