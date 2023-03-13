#pragma once

#include "DeviceBle.h"

using namespace std;

class DeviceBleScreenTouch : public DeviceBle
{
private:
	void SendDatetime();

public:
	DeviceBleScreenTouch(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);
};
