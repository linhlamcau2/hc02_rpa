#include "DeviceBleDoorSensor.h"
#include "Log.h"

DeviceBleDoorSensor::DeviceBleDoorSensor(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_DOOR_SENSOR, version)
{
	moduleDoorHangOn = new ModuleDoorHangOn(this, addr);
	moduleDoorStatus = new ModuleDoorStatus(this, addr);
	modulePinLevel = new ModulePinLevel(this, addr);
	modules.push_back(moduleDoorHangOn);
	modules.push_back(moduleDoorStatus);
	modules.push_back(modulePinLevel);
	powerSource = POWER_BATTERY;
}
