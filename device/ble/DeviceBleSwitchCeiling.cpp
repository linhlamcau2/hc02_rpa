#include "DeviceBleSwitchCeiling.h"
#include "Log.h"

DeviceBleSwitchCeiling::DeviceBleSwitchCeiling(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, uint8_t element)
	: DeviceBle(id, name, mac, data, addr, type, version)
{
	this->countElement = element;
	for (int i = 0; i < this->countElement; i++)
	{
		moduleButton = new ModuleButton(this, addr + i);
		modules.push_back(moduleButton);
	}
	moduleOnOff = new ModuleOnOff(this, addr);
	modules.push_back(moduleOnOff);
	moduleCountDownSwitch = new ModuleCountDownSwitch(this, addr);
	modules.push_back(moduleCountDownSwitch);
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);
	moduleAllRelay = new ModuleAllRelay(this, addr);
	modules.push_back(moduleAllRelay);
	powerSource = POWER_AC;
}

int DeviceBleSwitchCeiling::BuildTelemetryValue(Json::Value &pushDataValue)
{
	moduleOnOff->BuildTelemetryValue(pushDataValue);
	return CODE_OK;
}