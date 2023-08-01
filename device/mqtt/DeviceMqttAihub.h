#pragma once

#include "DeviceMqtt.h"

using namespace std;

class DeviceMqttAihub : public DeviceMqtt
{
public:
	DeviceMqttAihub(string id, string name, string mac, Json::Value &dataJson, uint16_t version);
};
