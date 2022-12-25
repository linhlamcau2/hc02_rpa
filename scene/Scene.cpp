#include "Scene.h"
#include <functional>
#include "TimerSchedule.h"
#include <Util.h>
#include <Log.h>

Scene::Scene(string id, string type, unsigned char repeater)
{
	this->id = id;
	this->type = type;
	this->repeater = repeater;
	this->startTime = -1;
	this->endTime = -1;
	count = 0;
	lastTimeActive = 0;
	timerRegisterIndex = 0;
}

Scene::Scene(string id, string type, unsigned char repeater, int startTime, int endTime)
{
	this->id = id;
	this->type = type;
	this->repeater = repeater;
	this->startTime = startTime;
	this->endTime = endTime;
	timerRegisterIndex = timerSchedule->RegisterTimer(startTime, bind(&Scene::Check, this));
	// timerSchedule->RegisterTimer(endTime, bind(&Scene::Check, this));
	count = 0;
	lastTimeActive = 0;
}

Scene::~Scene()
{
	LOGI("~Scene");
	for (auto &sceneInput : sceneInputList)
	{
		delete sceneInput;
	}
	for (auto &sceneOutput : sceneOutputList)
	{
		delete sceneOutput;
	}
	if (timerRegisterIndex > 0)
		timerSchedule->UnregisterTimer(timerRegisterIndex);
}

string Scene::GetId()
{
	return id;
}

void Scene::Check()
{
	bool checkSceneInputResult = false;
	int currentTimer = Util::GetCurrentTimer();
	int currentWeekDay = Util::GetCurrentWeekDay();
	LOGI("currentWeekDay: %d", currentWeekDay);
	LOGI("repeater: 0x%02X", repeater);
	if ((1 << currentWeekDay) & repeater)
	{
		LOGI("Check repeater day OK");
		if ((startTime < 0) || (endTime < 0) || (startTime <= currentTimer && currentTimer <= endTime) || (endTime <= startTime && currentTimer <= endTime) || (endTime <= startTime && startTime <= currentTimer))
		{
			LOGI("Check time OK");
			if (type == "or")
			{
				checkSceneInputResult = false;
				for (auto &sceneInput : sceneInputList)
				{
					if (sceneInput->Check())
					{
						checkSceneInputResult = true;
						break;
					}
				}
			}
			else if (type == "and")
			{
				checkSceneInputResult = true;
				for (auto &sceneInput : sceneInputList)
				{
					if (sceneInput->Check() == false)
					{
						checkSceneInputResult = false;
						break;
					}
				}
			}
		}
	}
	if (checkSceneInputResult)
	{
		LOGI("Do output scene id: %d", id);
		RunOutput();
		count++;
		lastTimeActive = time(NULL);
	}
}

void Scene::RunOutput()
{
	for (auto &sceneOutput : sceneOutputList)
	{
		sceneOutput->RunOutput();
	}
}

void Scene::AddSceneInput(SceneInput *sceneInput)
{
	sceneInputList.push_back(sceneInput);
}

void Scene::AddSceneOutput(SceneOutput *sceneOutput)
{
	sceneOutputList.push_back(sceneOutput);
}
