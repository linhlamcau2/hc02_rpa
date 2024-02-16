#include "DeviceBleSeftPowerRemote.h"
#include "BleProtocol.h"
#include "Log.h"

DeviceBleSeftPowerRemote::DeviceBleSeftPowerRemote(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, Device *parent)
    : DeviceBle(id, name, mac, data, addr, type, version)
{
    this->parent = parent;
    moduleButtonSeftPowerRemote = new ModuleButtonSeftPowerRemote(this, addr);
    modules.push_back(moduleButtonSeftPowerRemote);
    powerSource = POWER_BATTERY;
}

void DeviceBleSeftPowerRemote::SetParentDev(Device *parent)
{
    this->parent = parent;
}

Device *DeviceBleSeftPowerRemote::GetParent()
{
    return this->parent;
}
