#include "DeviceBleScreenTouch.h"
#include <thread>
#include "BleProtocol.h"
#include "Util.h"
#include "Log.h"
#include "Http.h"

DeviceBleScreenTouch::DeviceBleScreenTouch(string id, string name, string mac, string data, uint32_t addr, uint16_t version)
	: DeviceBle(id, name, mac, data, addr, BLE_AC_SCENE_SCREEN_TOUCH, version)
{
	moduleNotifyScene = new ModuleNotifyScene(this, addr);
	modules.push_back(moduleNotifyScene);
	powerSource = POWER_AC;
}
