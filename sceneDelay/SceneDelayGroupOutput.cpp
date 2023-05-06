#include "SceneDelayGroupOutput.h"
#include "Log.h"

SceneDelayGroupOutput::SceneDelayGroupOutput(Group *group, Json::Value &data, int delayTime)
{
	this->group = group;
	this->data = data;
	this->delayTime = delayTime;
}

SceneDelayGroupOutput::~SceneDelayGroupOutput()
{
	LOGI("~SceneDelayGroupOutput");
}

void SceneDelayGroupOutput::RunOutput()
{
	if (group)
	{
		sleep(delayTime);
		group->Do(data);
	}
}
