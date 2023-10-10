#include "DeviceBleRepeater.h"
#include "Log.h"

DeviceBleRepeater::DeviceBleRepeater(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, data, addr, type, version)
{
    powerSource = POWER_AC;
}