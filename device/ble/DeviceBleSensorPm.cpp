#include "DeviceBleSensorPm.h"
#include "Log.h"

DeviceBleSensorPm::DeviceBleSensorPm(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_PM_SENSOR, version)
{
	modulePmSensor = new ModulePmSensor(this, addr);
	modules.push_back(modulePmSensor);
	powerSource = POWER_BATTERY;
}
