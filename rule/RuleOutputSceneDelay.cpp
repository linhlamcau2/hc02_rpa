#include "RuleOutputSceneDelay.h"
#include "Log.h"

RuleOutputSceneDelay::RuleOutputSceneDelay(SceneDelay *sceneDelay, int delayTime)
{
	this->sceneDelay = sceneDelay;
	this->delayTime = delayTime;
}

RuleOutputSceneDelay::~RuleOutputSceneDelay()
{
	LOGI("~RuleOutputSceneDelay");
}

void RuleOutputSceneDelay::RunOutput()
{
	if (sceneDelay)
	{
		sleep(delayTime);
		sceneDelay->RunOutput();
	}
}
