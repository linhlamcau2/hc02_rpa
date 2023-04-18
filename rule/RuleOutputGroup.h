#pragma once

#include "json.h"
#include "Group.h"
#include "RuleOutput.h"

using namespace std;

class RuleOutputGroup : public RuleOutput
{
private:
	Group *group;
	int delayTime;
	Json::Value data;

public:
	RuleOutputGroup(Group *group, Json::Value &data, int delayTime);
	~RuleOutputGroup();

	void RunOutput();
};
