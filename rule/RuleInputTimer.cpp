#include "RuleInputTimer.h"
#include "Util.h"
#include "TimerSchedule.h"
#include "Log.h"

RuleInputTimer::RuleInputTimer(Rule *rule, int timer, int repeat)
{
	this->rule = rule;
	this->timer = timer;
	this->repeat = repeat;
	timerRegisterIndex = timerSchedule->RegisterTimer(timer, bind(&Rule::Check, rule));
}

RuleInputTimer::~RuleInputTimer()
{
	LOGI("~RuleInputTimer");
	if (timerRegisterIndex > 0)
		timerSchedule->UnregisterTimer(timerRegisterIndex);
}

bool RuleInputTimer::Check()
{
	if (!Util::HaveRTC())
	{
		LOGW("Not have RTC");
		return false;
	}
	int currentWeekDay = Util::GetCurrentWeekDay();
	LOGI("currentWeekDay : %d", currentWeekDay);
	LOGI("repeat : 0x%02X", repeat);
	LOGI("%s", rule->GetFirstRun() ? "true" : "false");
	return (Util::CheckDayInWeek(currentWeekDay, repeat) | (rule->GetFirstRun())) && timer == Util::GetCurrentTimer();
}
