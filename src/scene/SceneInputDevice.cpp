#include "SceneInputDevice.h"
#include <Device.h>
#include <Log.h>

SceneInputDevice::SceneInputDevice(Scene *scene, Device *device, Json::Value data)
{
	this->scene = scene;
	this->device = device;
	this->data = data;
	device->CheckData(data, isAvailable);
	if (device)
	{
		device->RegisterTrigger(this);
	}
}

SceneInputDevice::~SceneInputDevice()
{
	LOGI("~SceneInputDevice");
	if (device)
	{
		device->UnregisterTrigger(this);
	}
}

Json::Value *SceneInputDevice::GetData()
{
	return &data;
}

void SceneInputDevice::Trigger(bool value)
{
	LOGD("Trigger");
	isAvailable = value;
	if (isAvailable && scene)
	{
		scene->Check();
	}
}
