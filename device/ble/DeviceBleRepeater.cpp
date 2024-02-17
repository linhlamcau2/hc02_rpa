#include "DeviceBleRepeater.h"
#include "Log.h"

DeviceBleRepeater::DeviceBleRepeater(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version)
    : DeviceBle(id, name, mac, dataJson, addr, type, version)
{
    powerSource = POWER_AC;
}