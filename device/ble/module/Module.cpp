#include "Module.h"

Module::Module(Device *device, uint32_t addr)
{
	this->device = device;
	this->addr = addr;
}

Module::~Module()
{
}
