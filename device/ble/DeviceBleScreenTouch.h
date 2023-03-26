#pragma once

#include "DeviceBle.h"

using namespace std;

class DeviceBleScreenTouch : public DeviceBle
{
private:
	void SendDatetime();

public:
	DeviceBleScreenTouch(string id, string name, string mac, string data, uint32_t addr, uint16_t version);
};
