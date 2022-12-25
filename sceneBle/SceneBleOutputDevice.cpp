#include "SceneBleOutputDevice.h"
#include "Log.h"

SceneBleOutputDevice::SceneBleOutputDevice(Device *device, Json::Value data)
{
	this->device = device;
	this->data = data;
}

SceneBleOutputDevice::~SceneBleOutputDevice()
{
	LOGI("~SceneOutputDevice");
}

void SceneBleOutputDevice::RunOutput()
{
	if (device)
		device->Do(data);
}
