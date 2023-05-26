#include "DeviceBleSensorTempHum.h"
#include "Log.h"

DeviceBleSensorTempHum::DeviceBleSensorTempHum(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool isFavorite)
		: DeviceBle(id, name, mac, data, addr, BLE_TEMP_HUM_SENSOR, version, isFavorite)
{
	moduleTempHum = new ModuleTempHum(this, addr);
	modulePinLevel = new ModulePinLevel(this, addr);
	modules.push_back(moduleTempHum);
	modules.push_back(modulePinLevel);
	powerSource = POWER_BATTERY;
}
