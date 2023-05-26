#include "DeviceBlePirLightSensorDC.h"
#include "BleProtocol.h"
#include "Log.h"
#include "Util.h"

DeviceBlePirLightSensorDC::DeviceBlePirLightSensorDC(string id, string name, string mac, string data, uint32_t addr, uint16_t version, bool isFavorite)
		: DeviceBle(id, name, mac, data, addr, BLE_PIR_LIGHT_SENSOR_DC, version, isFavorite)
{
	modulePirLight = new ModulePirLight(this, addr);
	modulePirSensor = new ModulePirSensor(this, addr);
	moduleLightSensor = new ModuleLightSensor(this, addr);
	modulePinLevel = new ModulePinLevel(this, addr);
	moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
	modules.push_back(modulePirLight);
	modules.push_back(modulePirSensor);
	modules.push_back(moduleLightSensor);
	modules.push_back(modulePinLevel);
	modules.push_back(moduleTimeActionPir);
	powerSource = POWER_BATTERY;
}
