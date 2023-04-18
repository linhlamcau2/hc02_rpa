#include "RuleOutputDevice.h"
#include "Log.h"

RuleOutputDevice::RuleOutputDevice(Device *device, Json::Value &data, int delayTime)
{
	this->device = device;
	this->data = data;
	this->delayTime = delayTime;
}

RuleOutputDevice::~RuleOutputDevice()
{
	LOGI("~RuleOutputDevice");
}

void RuleOutputDevice::RunOutput()
{	
	if (device)
	{
		sleep(delayTime);
		device->DoJsonArrayV2(data);
		device->DoJsonArray(data);
	}
}
