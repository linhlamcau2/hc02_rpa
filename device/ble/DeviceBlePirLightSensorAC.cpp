#include "DeviceBlePirLightSensorAC.h"
#include "BleProtocol.h"
#include "Log.h"
#include "Util.h"

DeviceBlePirLightSensorAC::DeviceBlePirLightSensorAC(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint16_t version)
		: DeviceBle(id, name, mac, dataJson, addr, BLE_PIR_LIGHT_SENSOR_AC, version)
{
	modulePirLight = new ModulePirLight(this, addr);
	modulePirSensor = new ModulePirSensor(this, addr);
	moduleLightSensor = new ModuleLightSensor(this, addr);
	moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
	modules.push_back(modulePirLight);
	modules.push_back(modulePirSensor);
	modules.push_back(moduleLightSensor);
	modules.push_back(moduleTimeActionPir);
	powerSource = POWER_AC;
}
