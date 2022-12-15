#pragma once
#include "DeviceModbus.h"

using namespace std;

class DeviceModbusES_SM_TH_01 : public DeviceModbus
{
private:
public:
	DeviceModbusES_SM_TH_01(string id, string name, string mac, uint32_t addr);
};
