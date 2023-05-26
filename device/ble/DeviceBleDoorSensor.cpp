#include "DeviceBleDoorSensor.h"
#include "Log.h"

DeviceBleDoorSensor::DeviceBleDoorSensor(string id, string name, string mac, string data, uint32_t addr, uint16_t version, bool isFavorite)
		: DeviceBle(id, name, mac, data, addr, BLE_DOOR_SENSOR, version, isFavorite)
{
	moduleDoorHangOn = new ModuleDoorHangOn(this, addr);
	moduleDoorStatus = new ModuleDoorStatus(this, addr);
	modulePinLevel = new ModulePinLevel(this, addr);
	modules.push_back(moduleDoorHangOn);
	modules.push_back(moduleDoorStatus);
	modules.push_back(modulePinLevel);
	powerSource = POWER_BATTERY;
}
