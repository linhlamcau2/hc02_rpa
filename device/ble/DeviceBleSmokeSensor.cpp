#include "DeviceBleSmokeSensor.h"

DeviceBleSmokeSensor::DeviceBleSmokeSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint16_t version)
		: DeviceBle(id, name, mac, dataJson, addr, BLE_SMOKE_SENSOR, version)
{
	ModuleSmoke *moduleSmoke;
	moduleSmoke = new ModuleSmoke(this, addr);
	modules.push_back(moduleSmoke);
	powerSource = POWER_BATTERY;
}
