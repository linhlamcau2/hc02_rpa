#pragma once

#include "json.h"
#include "Group.h"
#include "SceneDelayOutput.h"

using namespace std;

class SceneDelayGroupOutput : public SceneDelayOutput
{
private:
	Group *group;
	int delayTime;
	Json::Value data;

public:
	SceneDelayGroupOutput(Group *group, Json::Value &data, int delayTime);
	~SceneDelayGroupOutput();

	void RunOutput();
};
