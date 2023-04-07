#include "DeviceBleLightOnoffHslModeRGB.h"
#include "Log.h"

DeviceBleLightOnoffHslModeRGB::DeviceBleLightOnoffHslModeRGB(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, data, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr);
	moduleModeRgb = new ModuleModeRgb(this, addr);
	moduleHsl = new ModuleHsl(this, addr);
	moduleOnoffHsl = new ModuleOnoffHsl(this, addr);
	moduleCallScene = new ModuleCallScene(this, addr);
	modules.push_back(moduleOnOff);
	modules.push_back(moduleModeRgb);
	modules.push_back(moduleHsl);
	modules.push_back(moduleOnoffHsl);
	modules.push_back(moduleCallScene);
	countElement = 2;
	powerSource = POWER_AC;
}
