#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCallScene.h"
#include "module/ModuleInputModuleInOut.h"
#include "module/ModuleModeInModuleInOut.h"
#include "module/ModuleADC.h"
#include "module/ModuleLinkInOut.h"
#include "module/ModuleDelta.h"
#include "module/ModuleStatusStartupRelay.h"

using namespace std;

class DeviceBleModuleInOut : public DeviceBle
{
private:
	uint8_t numInput;

	ModuleOnOff *moduleOnOff;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCallScene *moduleCallScene;
	ModuleInputModuleInOut *moduleInputModuleInOut;
	ModuleModeInModuleInOut *moduleModeInModuleInOut;
	ModuleADC *moduleADC;
	ModuleLinkInOut *moduleLinkInOut;
	ModuleDelta *moduleDelta;
	ModuleStatusStartupRelay *moduleStatusStartupRelay;

public:
	DeviceBleModuleInOut(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, uint8_t countElement, uint8_t numInput);
};
