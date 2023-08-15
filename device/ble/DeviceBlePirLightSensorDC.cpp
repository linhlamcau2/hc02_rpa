#include "DeviceBlePirLightSensorDC.h"
#include "BleProtocol.h"
#include "Log.h"
#include "Util.h"

DeviceBlePirLightSensorDC::DeviceBlePirLightSensorDC(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint16_t version)
		: DeviceBle(id, name, mac, dataJson, addr, BLE_PIR_LIGHT_SENSOR_DC, version)
{
	modulePirLight = new ModulePirLight(this, addr);
	modulePirSensor = new ModulePirSensor(this, addr);
	moduleLightSensor = new ModuleLightSensor(this, addr);
	moduleBatteryLevel = new ModuleBatteryLevel(this, addr);
	moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
	modules.push_back(modulePirLight);
	modules.push_back(modulePirSensor);
	modules.push_back(moduleLightSensor);
	modules.push_back(moduleBatteryLevel);
	modules.push_back(moduleTimeActionPir);
	powerSource = POWER_BATTERY;
}
