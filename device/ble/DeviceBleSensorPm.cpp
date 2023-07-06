#include "DeviceBleSensorPm.h"
#include "Log.h"

DeviceBleSensorPm::DeviceBleSensorPm(string id, string name, string mac, string data, uint32_t addr, uint16_t version)
	: DeviceBle(id, name, mac, data, addr, BLE_PM_SENSOR, version)
{
	modulePmSensor = new ModulePmSensor(this, addr);
	modules.push_back(modulePmSensor);
	moduleTempHum = new ModuleTempHum(this, addr);
	modules.push_back(moduleTempHum);
	powerSource = POWER_BATTERY;
}
