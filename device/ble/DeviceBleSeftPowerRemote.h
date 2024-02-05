#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"

using namespace std;

class DeviceBleSeftPowerRemote : public DeviceBle
{
private:
	Device *parent;
	ModuleButton *moduleButton[6];

public:
	DeviceBleSeftPowerRemote(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, Device *parent);
	~DeviceBleSeftPowerRemote();
	void SetParentDev(Device *parent);
	Device *GetParent();
};
