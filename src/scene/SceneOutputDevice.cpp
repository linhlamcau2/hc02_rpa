#include "SceneOutputDevice.h"
#include "Log.h"

SceneOutputDevice::SceneOutputDevice(Device *device, Json::Value data)
{
	this->device = device;
	this->data = data;
}

SceneOutputDevice::~SceneOutputDevice()
{
	LOGI("~SceneOutputDevice");
}

void SceneOutputDevice::RunOutput()
{
	if (device)
		device->Do(data);
}
