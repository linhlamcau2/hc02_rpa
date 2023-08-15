#pragma once

#include "DeviceZigbee.h"

using namespace std;

class DeviceZigbeeTuyaSensorMagnet : public DeviceZigbee
{
private:
public:
	DeviceZigbeeTuyaSensorMagnet(string id, string name, string mac, Json::Value &dataJson, uint16_t addr);
};
