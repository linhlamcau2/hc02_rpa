#include "RuleInputTimer.h"
#include "Util.h"
#include "TimerSchedule.h"
#include "Log.h"

RuleInputTimer::RuleInputTimer(Rule *rule, int timer, int repeat)
{
	this->rule = rule;
	this->timer = timer;
	this->repeat = repeat;
	if (rule)
		timerRegisterIndex = timerSchedule->RegisterTimer(timer, rule);
}

RuleInputTimer::~RuleInputTimer()
{
	LOGI("~RuleInputTimer");
	if (timerRegisterIndex > 0)
		timerSchedule->UnregisterTimer(timerRegisterIndex);
}

bool RuleInputTimer::Check()
{
	int currentWeekDay = Util::GetCurrentWeekDay();
	LOGI("currentWeekDay : %d", currentWeekDay);
	LOGI("repeat : 0x%02X", repeat);
	return ((1 << currentWeekDay) & repeat) && timer == Util::GetCurrentTimer();
}
