#include "Module.h"
#include "Device.h"
#include "Log.h"

Module::Module(Device *device, uint32_t addr)
{
	this->device = device;
	this->addr = addr;
}

Module::~Module()
{
}

bool Module::CheckAddr(uint32_t addr)
{
	return this->addr == addr;
}

void Module::CheckTrigger()
{
	LOGV("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

bool Module::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGW("CheckData not implement");
	return false;
}
