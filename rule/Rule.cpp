#include "Rule.h"
#include <functional>
#include "TimerSchedule.h"
#include "Util.h"
#include "Log.h"
#include "Db.h"
#ifdef ESP_PLATFORM
#include "Sntp.h"
#endif

Rule::Rule(string id, RuleType type, unsigned char repeater, bool isFirstRun, string name, uint16_t addr, Json::Value &ruleData) : Object(id, addr, name)
{
	this->type = type;
	this->repeater = repeater;
	this->startTime = -1;
	this->endTime = -1;
	this->ruleData = ruleData;
	this->isFirstRun = isFirstRun;
	isEnable = true;
	timerRegisterIndex = 0;
}

Rule::Rule(string id, RuleType type, unsigned char repeater, bool isFirstRun, string name, uint16_t addr, int startTime, int endTime, Json::Value &ruleData) : Object(id, addr, name)
{
	this->type = type;
	this->repeater = repeater;
	this->startTime = startTime;
	this->endTime = endTime;
	this->ruleData = ruleData;
	this->isFirstRun = isFirstRun;
	isEnable = true;
	timerRegisterIndex = timerSchedule->RegisterTimer(startTime, bind(&Rule::Check, this));
}

Rule::~Rule()
{
	LOGI("~Rule");
	for (auto &ruleInput : ruleInputList)
	{
		delete ruleInput;
	}
	for (auto &ruleOutput : ruleOutputList)
	{
		delete ruleOutput;
	}
	if (timerRegisterIndex > 0)
		timerSchedule->UnregisterTimer(timerRegisterIndex);
}

Json::Value Rule::GetRuleData()
{
	return ruleData;
}

void Rule::SetRuleData(Json::Value ruleData)
{
	this->ruleData = ruleData;
}

RuleType Rule::GetType()
{
	return type;
}

void Rule::UpdateFirstRun()
{
	isFirstRun = false;
	Json::Value ruleData = GetRuleData();
	ruleData["isFirstRun"] = false;
	database->RuleUpdateData(this, this->ruleData.toString());
}

void Rule::Check()
{
	if (isEnable)
	{
		bool checkRuleInputResult = false;
		int currentTimer = Util::GetCurrentTimer();
		int currentWeekDay = Util::GetCurrentWeekDay();
		if (Util::CheckDayInWeek(currentWeekDay, repeater) /*|| isFirstRun*/)
		{
			LOGD("Check repeater day OK");
			if ((startTime < 0) || (startTime <= currentTimer && currentTimer <= endTime) || (startTime == currentTimer))
			{
				LOGD("Check time OK");
				if (type == RULE_TYPE_OR || type == RULE_TYPE_TIME_OR)
				{
					checkRuleInputResult = false;
					for (auto &ruleInput : ruleInputList)
					{
						if (ruleInput->Check())
						{
							checkRuleInputResult = true;
							break;
						}
					}
				}
				else if (type == RULE_TYPE_AND || type == RULE_TYPE_TIME_AND)
				{
					checkRuleInputResult = true;
					for (auto &ruleInput : ruleInputList)
					{
						if (ruleInput->Check() == false)
						{
							checkRuleInputResult = false;
							break;
						}
					}
				}
			}
			if (((startTime <= currentTimer && currentTimer <= endTime) || (startTime <= currentTimer && endTime < 0)) && type == RULE_TYPE_TIME)
			{
				checkRuleInputResult = true;
			}
		}
		if (checkRuleInputResult)
		{
			LOGI("Do output rule id: %s", id.c_str());
			RunOutput();
			// UpdateFirstRun();
		}
	}
}

void Rule::RunOutput()
{
#ifdef ESP_PLATFORM
	if (Sntp::haveNtpTime())
	{

		for (auto &ruleOutput : ruleOutputList)
		{
			ruleOutput->RunOutput();
		}
	}
#else
	for (auto &ruleOutput : ruleOutputList)
	{
		ruleOutput->RunOutput();
	}
#endif
}

void Rule::AddRuleInput(RuleInput *ruleInput)
{
	ruleInputList.push_back(ruleInput);
}

void Rule::AddRuleOutput(RuleOutput *ruleOutput)
{
	ruleOutputList.push_back(ruleOutput);
}

void Rule::DelAllRuleInput()
{
	for (auto ruleInput : ruleInputList)
		delete ruleInput;
	ruleInputList.clear();
}

void Rule::DelAllRuleOutput()
{
	for (auto ruleOutput : ruleOutputList)
		delete ruleOutput;
	ruleOutputList.clear();
}

void Rule::UpdateData(Json::Value &ruleData)
{
	this->ruleData = ruleData;
}

bool Rule::GetStatus()
{
	return this->isEnable;
}

void Rule::SetStatus(bool enable)
{
	this->isEnable = enable;
}
