#pragma once

#include <json.h>
#include "Device.h"
#include "../rule/RuleOutput.h"

using namespace std;

class SceneBleOutputDevice : public RuleOutput
{
private:
	Device *device;
	Json::Value data;

public:
	SceneBleOutputDevice(Device *device, Json::Value data);
	~SceneBleOutputDevice();

	void RunOutput();
};