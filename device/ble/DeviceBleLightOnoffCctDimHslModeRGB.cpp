#include "DeviceBleLightOnoffCctDimHslModeRGB.h"
#include "Log.h"

DeviceBleLightOnoffCctDimHslModeRGB::DeviceBleLightOnoffCctDimHslModeRGB(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_ONOFF);
	moduleDim = new ModuleDim(this, addr);
	moduleModeRgb = new ModuleModeRgb(this, addr);
	moduleHsl = new ModuleHsl(this, addr);
	moduleCallScene = new ModuleCallScene(this, addr);
	moduleCct = new ModuleCct(this, addr + 1);
	modules.push_back(moduleHsl);
	modules.push_back(moduleOnOff);
	modules.push_back(moduleDim);
	modules.push_back(moduleModeRgb);
	modules.push_back(moduleCallScene);
	modules.push_back(moduleCct);
	countElement = 2;
	powerSource = POWER_AC;
}
