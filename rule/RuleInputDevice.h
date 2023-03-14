#pragma once

#include "RuleInput.h"
#include "Rule.h"
#include <functional>
#include "json.h"

using namespace std;

typedef function<void(bool)> DeviceRuleInputCallbackFunc;

class Device;
class RuleInputDevice : public RuleInput
{
private:
	Device *device;
	Json::Value data;

public:
	RuleInputDevice(Rule *rule, Device *device, Json::Value data);
	~RuleInputDevice();
	Json::Value *GetData();
	void Trigger(bool value);
};