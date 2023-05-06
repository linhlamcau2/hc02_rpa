#include "SceneDelay.h"
#include <functional>
#include "Util.h"
#include "Log.h"

SceneDelay::SceneDelay(string id, uint32_t addr, string name, Json::Value &data) : Object(id, addr, name)
{
	this->data = data;
}

SceneDelay::~SceneDelay()
{
	LOGI("~SceneDelay");
	for (auto &sceneDelayOutput : sceneDelayOutputs)
	{
		delete sceneDelayOutput;
	}
}

Json::Value SceneDelay::GetData()
{
	return data;
}

void SceneDelay::RunOutput()
{
	for (auto &sceneDelayOutput : sceneDelayOutputs)
	{
		sceneDelayOutput->RunOutput();
	}
}

void SceneDelay::AddSceneDelayOutput(SceneDelayOutput *output)
{
	sceneDelayOutputs.push_back(output);
}

void SceneDelay::DelAllSceneDelayOutput()
{
	sceneDelayOutputs.clear();
}

void SceneDelay::UpdateData(string data)
{
	Json::Value sceneDelayData;
	sceneDelayData.parse(data);
	this->data = sceneDelayData;
}
