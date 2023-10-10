#pragma once

#include "DeviceBle.h"

using namespace std;

class DeviceBleRepeater : public DeviceBle
{
public:
    DeviceBleRepeater(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};