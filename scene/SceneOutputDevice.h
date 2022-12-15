#pragma once

#include <json.h>
#include "Device.h"
#include "SceneOutput.h"

using namespace std;

class SceneOutputDevice : public SceneOutput
{
private:
	Device *device;
	Json::Value data;

public:
	SceneOutputDevice(Device *device, Json::Value data);
	~SceneOutputDevice();

	void RunOutput();
};