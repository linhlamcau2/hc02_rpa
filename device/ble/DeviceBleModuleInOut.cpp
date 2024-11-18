#include "DeviceBleModuleInOut.h"

DeviceBleModuleInOut::DeviceBleModuleInOut(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement, uint8_t numInput)
	: DeviceBle(id, name, mac, dataJson, addr, type, version)
{
	this->countElement = countElement;
	this->numInput = numInput;
	for (int i = 0; i < countElement; i++)
	{
		moduleOnOff = new ModuleOnOff(this, addr + i, KEY_ATTRIBUTE_BUTTON, i);
		modules.push_back(moduleOnOff);
		moduleLinkInOut = new ModuleLinkInOut(this, addr, KEY_ATTRIBUTE_LINK_INOUT, i);
		modules.push_back(moduleLinkInOut);
	}
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);
	moduleCallScene = new ModuleCallScene(this, addr);
	modules.push_back(moduleCallScene);

	moduleADC = new ModuleADC(this, addr);
	for (int i = 1; i <= numInput; i++)
	{
		moduleInputModuleInOut = new ModuleInputModuleInOut(this, addr, KEY_ATTRIBUTE_INPUT_MODULE_INOUT, i);
		modules.push_back(moduleInputModuleInOut);
		moduleModeInModuleInOut = new ModuleModeInModuleInOut(this, addr, KEY_ATTRIBUTE_MODE_MODULE_INOUT, i);
		modules.push_back(moduleModeInModuleInOut);
	}
	powerSource = POWER_AC;
}
