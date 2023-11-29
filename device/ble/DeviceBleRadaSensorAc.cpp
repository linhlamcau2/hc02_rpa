
#include "DeviceBleRadaSensorAc.h"
#include "BleProtocol.h"
#include "Log.h"
#include "Util.h"

DeviceBleRadaSensorAc::DeviceBleRadaSensorAc(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, type, version)
{
	modulePirLight = new ModulePirLight(this, addr);
	modulePirSensor = new ModulePirSensor(this, addr);
	moduleLightSensor = new ModuleLightSensor(this, addr);
	moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
    moduleModeActionPir = new ModuleModeActionPir(this, addr);
	moduleSensiPir = new ModuleSensiPir(this, addr);
    moduleOnOff = new ModuleOnOff(this, addr);
	moduleDistance = new ModuleDistance(this, addr);
	modulePirLightSensorStartup = new ModulePirLightSensorStartup(this, addr);
	modules.push_back(modulePirLight);
	modules.push_back(modulePirSensor);
	modules.push_back(moduleLightSensor);
	modules.push_back(moduleTimeActionPir);
    modules.push_back(moduleModeActionPir);
	modules.push_back(moduleSensiPir);
    modules.push_back(moduleOnOff);
	modules.push_back(moduleDistance);
	modules.push_back(modulePirLightSensorStartup);
	powerSource = POWER_AC;
}
