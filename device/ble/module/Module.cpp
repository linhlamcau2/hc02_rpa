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


void Module::CheckTrigger(Json::Value &data)
{
	LOGV("CheckTrigger %s", data.toString().c_str());
}

bool Module::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGW("CheckData not implement");
	return false;
}
