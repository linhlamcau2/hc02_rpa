#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDim.h"
#include "module/ModuleOnoffCctDim.h"
#include "module/ModuleCallScene.h"
#include "element/ElementCct.h"

using namespace std;

class DeviceBleLightOnoffCctDim : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleDim *moduleDim;
	ModuleOnoffCctDim *moduleOnoffCctDim;
	ModuleCallScene *moduleCallScene;
	ElementCct *elementCct;

public:
	DeviceBleLightOnoffCctDim(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
};