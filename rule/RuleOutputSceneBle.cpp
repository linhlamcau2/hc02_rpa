#include "RuleOutputSceneBle.h"
#include "Log.h"

RuleOutputSceneBle::RuleOutputSceneBle(SceneBle *sceneBle, int delayTime)
{
	this->sceneBle = sceneBle;
	this->delayTime = delayTime;
}

RuleOutputSceneBle::~RuleOutputSceneBle()
{
	LOGI("~RuleOutputSceneBle");
}

void RuleOutputSceneBle::RunOutput()
{
	if (sceneBle)
	{
		sleep(delayTime);
		sceneBle->Do();
	}
}
