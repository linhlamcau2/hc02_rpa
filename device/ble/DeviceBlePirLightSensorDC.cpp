#include "DeviceBlePirLightSensorDC.h"
#include "BleProtocol.h"
#include "Log.h"
#include "Util.h"

DeviceBlePirLightSensorDC::DeviceBlePirLightSensorDC(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, type, version)
{
	modulePirLight = new ModulePirLight(this, addr);
	modulePirSensor = new ModulePirSensor(this, addr);
	moduleLightSensor = new ModuleLightSensor(this, addr);
	modulePinLevel = new ModulePinLevel(this, addr);
	moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
	moduleSensiPir = new ModuleSensiPir(this, addr);
	modules.push_back(modulePirLight);
	modules.push_back(modulePirSensor);
	modules.push_back(moduleLightSensor);
	modules.push_back(modulePinLevel);
	modules.push_back(moduleTimeActionPir);
	modules.push_back(moduleSensiPir);
	powerSource = POWER_BATTERY;
}
