#pragma once

#include "SceneInput.h"
#include "Scene.h"
#include <functional>
#include <json.h>

using namespace std;

class SceneInputTimer : public SceneInput
{
private:
	int timer;
	int repeat;
	int timerRegisterIndex;

public:
	SceneInputTimer(Scene *scene, int timer, int repeat);
	~SceneInputTimer();
	bool Check();
};