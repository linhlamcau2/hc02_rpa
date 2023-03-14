#include "DeviceBleLightOnoffHslModeRGB.h"
#include "Log.h"

DeviceBleLightOnoffHslModeRGB::DeviceBleLightOnoffHslModeRGB(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr);
	moduleModeRgb = new ModuleModeRgb(this, addr);
	moduleHsl = new ModuleHsl(this, addr);
	modules.push_back(moduleOnOff);
	modules.push_back(moduleModeRgb);
	modules.push_back(moduleHsl);
	countElement = 2;
	powerSource = POWER_AC;
}
