#include "RuleOutputDevice.h"
#include "Log.h"

RuleOutputDevice::RuleOutputDevice(Device *device, Json::Value &data)
{
	this->device = device;
	this->data = data;
}

RuleOutputDevice::~RuleOutputDevice()
{
	LOGI("~RuleOutputDevice");
}

void RuleOutputDevice::RunOutput()
{
	
	if (device)
	{
		device->DoJsonArrayV2(data);
		device->DoJsonArray(data);
	}
}
