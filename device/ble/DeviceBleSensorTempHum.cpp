#include "DeviceBleSensorTempHum.h"
#include "Log.h"

DeviceBleSensorTempHum::DeviceBleSensorTempHum(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, dataJson, addr, BLE_TEMP_HUM_SENSOR, version)
{
	moduleTempHum = new ModuleTempHum(this, addr);
	moduleBatteryLevel = new ModuleBatteryLevel(this, addr);
	modules.push_back(moduleTempHum);
	modules.push_back(moduleBatteryLevel);
	powerSource = POWER_BATTERY;
}
