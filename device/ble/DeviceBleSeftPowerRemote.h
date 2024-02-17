#pragma once

#include "DeviceBle.h"
#include "module/ModuleButtonSeftPowerRemote.h"

using namespace std;

class DeviceBleSeftPowerRemote : public DeviceBle
{
private:
	Device *parent;
	ModuleButtonSeftPowerRemote *moduleButtonSeftPowerRemote;

public:
	DeviceBleSeftPowerRemote(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version, Device *parent);
	void SetParentDev(Device *parent);
	Device *GetParent();
};
