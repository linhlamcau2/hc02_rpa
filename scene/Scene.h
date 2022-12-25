#pragma once

#include <string>
#include <vector>
#include "SceneInput.h"
#include "SceneOutput.h"

using namespace std;

typedef enum
{
	SCENE_TYPE_OR = 0,
	SCENE_TYPE_AND
} SceneType;

typedef enum
{
	SCENE_MODE_ALL_DAY = 0xFE,
	SCENE_MODE_WORK_DAY = 0xF1,
	SCENE_MODE_WEEKEND_DAY = 0x06,
} SceneMode;

class Scene
{
private:
	string id;
	string type;
	unsigned char repeater;
	bool fullDay;
	int startTime;
	int endTime;
	int count;
	time_t lastTimeActive;

	bool isAvailable;
	int timerRegisterIndex;

	vector<SceneInput *> sceneInputList;
	vector<SceneOutput *> sceneOutputList;

public:
	Scene(string id, string type, unsigned char repeater);
	Scene(string id, string type, unsigned char repeater, int startTime, int endTime);
	~Scene();

	string GetId();
	void AddSceneInput(SceneInput *sceneInput);
	void AddSceneOutput(SceneOutput *sceneOutput);
	void Check();
	void RunOutput();
};