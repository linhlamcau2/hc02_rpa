#include "SceneOutputGroup.h"
#include "Log.h"

SceneOutputGroup::SceneOutputGroup(Group *group, Json::Value data)
{
	this->group = group;
	this->data = data;
}

SceneOutputGroup::~SceneOutputGroup()
{
	LOGI("~SceneOutputGroup");
}

void SceneOutputGroup::RunOutput()
{
	if (group)
	{
		group->Do(data);
	}
}
