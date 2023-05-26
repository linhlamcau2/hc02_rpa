#include "DeviceBleSensorPm.h"
#include "Log.h"

DeviceBleSensorPm::DeviceBleSensorPm(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite)
	: DeviceBle(id, name, mac, data, addr, BLE_PM_SENSOR, version, isFavorite)
{
	modulePmSensor = new ModulePmSensor(this, addr);
	modules.push_back(modulePmSensor);

	moduleTempHum = new ModuleTempHum(this, addr);
	modules.push_back(moduleTempHum);

	powerSource = POWER_BATTERY;
}
