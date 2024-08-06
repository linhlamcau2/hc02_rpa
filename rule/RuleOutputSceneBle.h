#pragma once

#include "json.h"
#include "SceneBle.h"
#include "RuleOutput.h"

using namespace std;

class RuleOutputSceneBle : public RuleOutput
{
private:
	SceneBle *sceneBle;
	Json::Value data;

public:
	RuleOutputSceneBle(SceneBle *sceneBle);
	~RuleOutputSceneBle();

	void RunOutput();
};