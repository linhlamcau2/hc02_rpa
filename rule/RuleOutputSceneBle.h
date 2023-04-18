#pragma once

#include "json.h"
#include "SceneBle.h"
#include "RuleOutput.h"

using namespace std;

class RuleOutputSceneBle : public RuleOutput
{
private:
	SceneBle *sceneBle;
	int delayTime;
	Json::Value data;

public:
	RuleOutputSceneBle(SceneBle *sceneBle, int delayTime);
	~RuleOutputSceneBle();

	void RunOutput();
};