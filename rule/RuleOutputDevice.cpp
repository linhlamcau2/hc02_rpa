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
		device->DoJsonArray(data);
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
		device->DoJsonArrayV2(data);
#endif // CONFIG_USE_MESSAGE_FORMAT_V2
	}
}
