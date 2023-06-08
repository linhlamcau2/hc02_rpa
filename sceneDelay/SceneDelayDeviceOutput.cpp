#include "SceneDelayDeviceOutput.h"
#include "Log.h"

SceneDelayDeviceOutput::SceneDelayDeviceOutput(Device *device, Json::Value &data, int delayTime)
{
	this->device = device;
	this->data = data;
	this->delayTime = delayTime;
}

SceneDelayDeviceOutput::~SceneDelayDeviceOutput()
{
	LOGI("~RuleOutputDevice");
}

void SceneDelayDeviceOutput::RunOutput()
{
	if (device)
	{
		sleep(delayTime);
		device->DoJsonArray(data);
	}
}
