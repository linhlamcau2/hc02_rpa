#pragma once

#include <json.h>
#include "Group.h"
#include "SceneOutput.h"

using namespace std;

class SceneOutputGroup : public SceneOutput
{
private:
	Group *group;
	Json::Value data;

public:
	SceneOutputGroup(Group *group, Json::Value data);
	~SceneOutputGroup();

	void RunOutput();
};