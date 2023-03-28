#include "Rule.h"
#include <functional>
#include "TimerSchedule.h"
#include "Util.h"
#include "Log.h"

Rule::Rule(string id, string name, int addr, string type, unsigned char repeater) : Object(id, addr, name)
{
	this->type = type;
	this->repeater = repeater;
	this->startTime = -1;
	this->endTime = -1;
	count = 0;
	lastTimeActive = 0;
	timerRegisterIndex = 0;
}

Rule::Rule(string id, string name, int addr, string type, unsigned char repeater, int startTime, int endTime) : Object(id, addr, name)
{
	this->type = type;
	this->repeater = repeater;
	this->startTime = startTime;
	this->endTime = endTime;
	timerRegisterIndex = timerSchedule->RegisterTimer(startTime, bind(&Rule::Check, this));
	// timerSchedule->RegisterTimer(endTime, bind(&Rule::Check, this));
	count = 0;
	lastTimeActive = 0;
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

void Rule::Check()
{
	if (isEnable)
	{
		bool checkRuleInputResult = false;
		int currentTimer = Util::GetCurrentTimer();
		int currentWeekDay = Util::GetCurrentWeekDay();
		LOGI("currentWeekDay : %d", currentWeekDay);
		LOGI("currenWeekDay convert: %d", Util::ConvertWeekDayToIntCompare(currentWeekDay));
		LOGI("repeater : 0x%02X", repeater);
		if (Util::ConvertWeekDayToIntCompare(currentWeekDay) & repeater)
		{
			LOGI("Check repeater day OK");
			if ((startTime < 0) || (endTime < 0) || (startTime <= currentTimer && currentTimer <= endTime) || (endTime <= startTime && currentTimer <= endTime) || (endTime <= startTime && startTime <= currentTimer))
			{
				LOGI("Check time OK");
				if (type == "or")
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
				else if (type == "and")
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
		}
		if (checkRuleInputResult)
		{
			LOGI("Do output rule id: %s", id.c_str());
			RunOutput();
			count++;
			lastTimeActive = time(NULL);
		}
	}
}

void Rule::RunOutput()
{
	for (auto &ruleOutput : ruleOutputList)
	{
		ruleOutput->RunOutput();
	}
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
	ruleInputList.clear();
}

void Rule::DelAllRuleOutput()
{
	ruleOutputList.clear();
}

void Rule::UpdateData(string data)
{
	this->data = data;
}

string Rule::GetData()
{
	return data;
}

bool Rule::GetStatus()
{
	return this->isEnable;
}

void Rule::SetStatus(bool enable)
{
	this->isEnable = enable;
}