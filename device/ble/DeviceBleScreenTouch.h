#pragma once

#include "DeviceBle.h"
#include "module/ModuleNotifyScene.h"

using namespace std;

class DeviceBleScreenTouch : public DeviceBle
{
private:
	ModuleNotifyScene *moduleNotifyScene;

public:
	DeviceBleScreenTouch(string id, string name, string mac, string data, uint32_t addr, uint16_t version, bool isFavorite);
};
