#include "DeviceBleSmokeSensor.h"
#include "Log.h"

DeviceBleSmokeSensor::DeviceBleSmokeSensor(string id, string name, string mac, string data, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, BLE_SMOKE_SENSOR, version)
{
	moduleSmoke = new ModuleSmoke(this, addr);
	modules.push_back(moduleSmoke);
	powerSource = POWER_BATTERY;
}
