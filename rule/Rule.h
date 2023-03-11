#pragma once

#include <string>
#include <vector>
#include "RuleInput.h"
#include "RuleOutput.h"

#define EVENT_TRIGGER		"EVENT_TRIGGER"
#define COUNTDOWN			"COUNTDOWN"

using namespace std;

typedef enum
{
	SCENE_TYPE_OR = 0,
	SCENE_TYPE_AND
} RuleType;

typedef enum
{
	SCENE_MODE_ALL_DAY = 0xFE,
	SCENE_MODE_WORK_DAY = 0xF1,
	SCENE_MODE_WEEKEND_DAY = 0x06,
} RuleMode;

class Rule
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
	string cmd;
	bool enbale;

	vector<RuleInput *> ruleInputList;
	vector<RuleOutput *> ruleOutputList;

public:
	Rule(string id, string type, unsigned char repeater);
	Rule(string id, string type, unsigned char repeater, int startTime, int endTime, string cmd, bool enable);
	~Rule();

	bool isEnable;
	string GetId();
	string GetCmd();
	void AddRuleInput(RuleInput *ruleInput);
	void AddRuleOutput(RuleOutput *ruleOutput);
	void DelAllRuleInput();
	void DelAllRuleOutput();
	void Check();
	void RunOutput();
};
