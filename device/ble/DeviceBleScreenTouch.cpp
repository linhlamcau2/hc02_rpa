#include "DeviceBleScreenTouch.h"

DeviceBleScreenTouch::DeviceBleScreenTouch(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint16_t version)
	: DeviceBle(id, name, mac, dataJson, addr, BLE_AC_SCENE_SCREEN_TOUCH, version)
{
	moduleNotifyScene = new ModuleNotifyScene(this, addr);
	modules.push_back(moduleNotifyScene);
	powerSource = POWER_AC;
}
