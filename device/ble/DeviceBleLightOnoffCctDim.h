#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDim.h"
#include "module/ModuleCallScene.h"
#include "module/ModuleCct.h"

using namespace std;

class DeviceBleLightOnoffCctDim : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleDim *moduleDim;
	ModuleCallScene *moduleCallScene;
	ModuleCct *moduleCct;

public:
	DeviceBleLightOnoffCctDim(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
};