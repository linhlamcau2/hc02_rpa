#include "RuleOutputSceneBle.h"
#include "Log.h"

RuleOutputSceneBle::RuleOutputSceneBle(SceneBle *sceneBle)
{
	this->sceneBle = sceneBle;
}

RuleOutputSceneBle::~RuleOutputSceneBle()
{
	LOGI("~RuleOutputSceneBle");
}

void RuleOutputSceneBle::RunOutput()
{
	if (sceneBle)
	{
		sceneBle->Do();
	}
}
