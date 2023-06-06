#include "RuleOutputGroup.h"
#include "Log.h"

RuleOutputGroup::RuleOutputGroup(Group *group, Json::Value &data, int delayTime)
{
	this->group = group;
	this->data = data;
	this->delayTime = delayTime;
}

RuleOutputGroup::~RuleOutputGroup()
{
	LOGI("~RuleOutputGroup");
}

void RuleOutputGroup::RunOutput()
{
	if (group)
	{
		group->Do(data, true);
	}
}
