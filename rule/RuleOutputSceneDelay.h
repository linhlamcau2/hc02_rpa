#pragma once

#include "json.h"
#include "SceneDelay.h"
#include "RuleOutput.h"

using namespace std;

class RuleOutputSceneDelay : public RuleOutput
{
private:
	SceneDelay *sceneDelay;
	int delayTime;
	Json::Value data;

public:
	RuleOutputSceneDelay(SceneDelay *sceneDelay, int delayTime);
	~RuleOutputSceneDelay();

	void RunOutput();
};