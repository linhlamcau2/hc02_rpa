#include "DeviceMqttAihub.h"
#include "Log.h"

DeviceMqttAihub::DeviceMqttAihub(string id, string name, string mac, string data, uint16_t version)
		: DeviceMqtt(id, name, mac, data, 0, MQTT_AI_HUB, version)
{
	// TODO: parse data to get function list
}
