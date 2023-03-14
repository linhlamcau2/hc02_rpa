#include "Element.h"

Element::Element(Device *device, uint32_t addr)
{
	this->device = device;
	this->addr = addr;
}

bool Element::CheckAddr(uint32_t addr)
{
	return this->addr == addr;
}
