#include "SceneInputTimer.h"
#include "Util.h"
#include "TimerSchedule.h"
#include <Log.h>

SceneInputTimer::SceneInputTimer(Scene *scene, int timer, int repeat)
{
	this->scene = scene;
	this->timer = timer;
	this->repeat = repeat;
	timerRegisterIndex = timerSchedule->RegisterTimer(timer, bind(&Scene::Check, scene));
}

SceneInputTimer::~SceneInputTimer()
{
	LOGI("~SceneInputTimer");
	if (timerRegisterIndex > 0)
		timerSchedule->UnregisterTimer(timerRegisterIndex);
}

bool SceneInputTimer::Check()
{
	int currentWeekDay = Util::GetCurrentWeekDay();
	LOGI("currentWeekDay: %d", currentWeekDay);
	LOGI("repeat: 0x%02X", repeat);
	return ((1 << currentWeekDay) & repeat) && timer == Util::GetCurrentTimer();
}