#include "DeviceBleSeftPowerRemote.h"
#include "BleProtocol.h"
#include "Log.h"

DeviceBleSeftPowerRemote::DeviceBleSeftPowerRemote(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, Device *parent)
    : DeviceBle(id, name, mac, data, addr, type, version)
{
    this->parent = parent;
    for (int i = 0; i < 6; i++)
    {
        moduleButton[i] = new ModuleButton(this, addr, i);
        modules.push_back(moduleButton[i]);
    }
    powerSource = POWER_BATTERY;
}

DeviceBleSeftPowerRemote::~DeviceBleSeftPowerRemote()
{
    if (this->parent)
    {
        if (bleProtocol)
        {
            bleProtocol->ResetSeftPowerRemote(this->parent->GetAddr(), addr);
        }
        else
            LOGW("Ble protocol null");
    }
    else
        LOGW("parent device null");
}

void DeviceBleSeftPowerRemote::SetParentDev(Device *parent)
{
    this->parent = parent;
}

Device *DeviceBleSeftPowerRemote::GetParent()
{
    return this->parent;
}
