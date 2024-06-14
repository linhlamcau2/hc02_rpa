#include "DeviceBleDoorSensor.h"
#include "Log.h"

DeviceBleDoorSensor::DeviceBleDoorSensor(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	moduleDoorHangOn = new ModuleDoorHangOn(this, addr);
	moduleDoorStatus = new ModuleDoorStatus(this, addr);
	moduleBatteryLevel = new ModuleBatteryLevel(this, addr);
	modules.push_back(moduleDoorHangOn);
	modules.push_back(moduleDoorStatus);
	modules.push_back(moduleBatteryLevel);
	powerSource = POWER_BATTERY;
}
