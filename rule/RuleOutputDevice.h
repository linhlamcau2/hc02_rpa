#pragma once

#include "json.h"
#include "Device.h"
#include "RuleOutput.h"

using namespace std;

class RuleOutputDevice : public RuleOutput
{
private:
	Device *device;
	int delayTime;
	Json::Value data;

public:
	RuleOutputDevice(Device *device, Json::Value &data, int delayTime);
	~RuleOutputDevice();

	void RunOutput();
};
