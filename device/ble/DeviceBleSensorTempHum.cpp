#include "DeviceBleSensorTempHum.h"
#include "Log.h"

DeviceBleSensorTempHum::DeviceBleSensorTempHum(string id, string name, string mac, string data, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, BLE_TEMP_HUM_SENSOR, version)
{
	moduleTempHum = new ModuleTempHum(this, addr);
	modulePinLevel = new ModulePinLevel(this, addr);
	modules.push_back(moduleTempHum);
	modules.push_back(modulePinLevel);
	powerSource = POWER_BATTERY;
}
