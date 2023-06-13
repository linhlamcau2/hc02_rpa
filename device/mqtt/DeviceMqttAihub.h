#pragma once

#include "DeviceMqtt.h"
#include "function/FunctionZone.h"

using namespace std;

class DeviceMqttAihub : public DeviceMqtt
{
private:
public:
	DeviceMqttAihub(string id, string name, string mac, string data, uint16_t version);
};
