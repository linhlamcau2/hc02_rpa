#include "Element.h"
#include "Device.h"
#include "Log.h"

Element::Element(Device *device, uint32_t addr)
{
	this->device = device;
	this->addr = addr;
}

Element::~Element()
{
}

bool Element::CheckAddr(uint32_t addr)
{
	return this->addr == addr;
}

void Element::CheckTrigger()
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

bool Element::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGW("CheckData not implement");
	return false;
}
