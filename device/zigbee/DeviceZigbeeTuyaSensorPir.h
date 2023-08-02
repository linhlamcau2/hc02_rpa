#pragma once

#include "DeviceZigbee.h"

using namespace std;

class DeviceZigbeeTuyaSensorPir : public DeviceZigbee
{
private:
public:
	DeviceZigbeeTuyaSensorPir(string id, string name, string mac, Json::Value &dataJson, uint32_t addr);
};
