#pragma once

#include "json.h"
#include "Device.h"
#include "SceneDelayOutput.h"

using namespace std;

class SceneDelayDeviceOutput : public SceneDelayOutput
{
private:
	Device *device;
	int delayTime;
	Json::Value data;

public:
	SceneDelayDeviceOutput(Device *device, Json::Value &data, int delayTime);
	~SceneDelayDeviceOutput();

	void RunOutput();
};
