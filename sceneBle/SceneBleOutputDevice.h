#pragma once

#include <json.h>
#include "Device.h"
#include "SceneOutput.h"

using namespace std;

class SceneBleOutputDevice : public SceneOutput
{
private:
	Device *device;
	Json::Value data;

public:
	SceneBleOutputDevice(Device *device, Json::Value data);
	~SceneBleOutputDevice();

	void RunOutput();
};