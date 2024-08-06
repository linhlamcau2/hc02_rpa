#include "DeviceMqttAihub.h"

DeviceMqttAihub::DeviceMqttAihub(string id, string name, string mac, Json::Value &dataJson, uint16_t version)
		: DeviceMqtt(id, name, mac, dataJson, 0, MQTT_AI_HUB, version)
{
}
