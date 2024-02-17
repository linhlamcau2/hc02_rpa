#include "Module.h"
#include "Device.h"
#include "Log.h"
#include <thread>

Module::Module(Device *device, uint16_t addr, uint32_t index)
{
	this->device = device;
	this->addr = addr;
	this->index = index;
}

Module::~Module()
{
}

bool Module::CheckAddr(uint16_t addr)
{
	return this->addr == addr;
}

static void CheckInputRuleDevice(void *data)
{
	Device *device = (Device *)data;
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (device->CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

void Module::CheckTrigger()
{
	LOGV("CheckTrigger");
#ifdef ESP_PLATFORM
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
#else
	thread checkOnlineThread(CheckInputRuleDevice, this->device);
	checkOnlineThread.detach();
#endif
}

bool Module::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGW("CheckData not implement");
	return false;
}
