#include "DeviceBleSmokeSensor.h"
#include "Log.h"

DeviceBleSmokeSensor::DeviceBleSmokeSensor(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite)
		: DeviceBle(id, name, mac, data, addr, BLE_SMOKE_SENSOR, version, isFavorite)
{
	moduleSmoke = new ModuleSmoke(this, addr);
	modules.push_back(moduleSmoke);
	powerSource = POWER_BATTERY;
}
