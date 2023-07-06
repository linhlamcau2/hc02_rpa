#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleSwitchScene6ACRgb : public DeviceBle
{
private:
	ModuleRgb *moduleRgb[6];
	ModuleButton *moduleButton[6];
	string idButton[6];

public:
	DeviceBleSwitchScene6ACRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
	bool CheckId(string id);
	void InputData(uint8_t *data, int len, uint32_t addr);
	int Do(Json::Value &dataValue, string id);
};
