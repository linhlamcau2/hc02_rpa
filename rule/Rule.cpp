#include "Rule.h"
#include <functional>
#include "TimerSchedule.h"
#include "Util.h"
#include "Log.h"
#ifdef ESP_PLATFORM
#include "Sntp.h"
#endif

Rule::Rule(string id, string type, unsigned char repeater, string name, uint32_t addr, Json::Value &ruleData) : Object(id, addr, name)
{
	this->type = type;
	this->repeater = repeater;
	this->startTime = -1;
	this->endTime = -1;
	this->ruleData = ruleData;
	timerRegisterIndex = 0;
}

Rule::Rule(string id, string type, unsigned char repeater, string name, uint32_t addr, int startTime, int endTime, Json::Value &ruleData) : Object(id, addr, name)
{
	this->type = type;
	this->repeater = repeater;
	this->startTime = startTime;
	this->endTime = endTime;
	this->ruleData = ruleData;
	timerRegisterIndex = timerSchedule->RegisterTimer(startTime, bind(&Rule::Check, this));
	// timerSchedule->RegisterTimer(endTime, bind(&Rule::Check, this));
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

string Rule::GetType()
{
	return type;
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
			if ((startTime < 0) || (endTime < 0) || (startTime <= currentTimer && currentTimer <= endTime))
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
