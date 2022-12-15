#pragma once

#include "SceneInput.h"
#include "Scene.h"
#include <functional>
#include <json.h>

using namespace std;

typedef function<void(bool)> DeviceSceneInputCallbackFunc;

class Device;
class SceneInputDevice : public SceneInput
{
private:
	Device *device;
	Json::Value data;

public:
	SceneInputDevice(Scene *scene, Device *device, Json::Value data);
	~SceneInputDevice();
	Json::Value *GetData();
	void Trigger(bool value);
};