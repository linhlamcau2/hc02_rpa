#include "DeviceBlePirLightSensorDC.h"
#include "BleProtocol.h"
#include "Log.h"
#include "Util.h"

DeviceBlePirLightSensorDC::DeviceBlePirLightSensorDC(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_PIR_LIGHT_SENSOR_DC, version)
{
	modulePirSensor = new ModulePirSensor(this, addr);
	moduleLightSensor = new ModuleLightSensor(this, addr);
	modulePinLevel = new ModulePinLevel(this, addr);
	moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
	modules.push_back(modulePirSensor);
	modules.push_back(moduleLightSensor);
	modules.push_back(modulePinLevel);
	modules.push_back(moduleTimeActionPir);
	powerSource = POWER_BATTERY;
}
