#pragma once

#include <string>
#include <vector>
#include "Object.h"
#include "RuleInput.h"
#include "RuleOutput.h"

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

class Rule : public Object
{
private:
	string type;
	string name;
	string data;
	
	unsigned char repeater;
	bool fullDay;
	int startTime;
	int endTime;
	int count;
	time_t lastTimeActive;

	bool isAvailable;
	bool isEnable;
	int timerRegisterIndex;

	vector<RuleInput *> ruleInputList;
	vector<RuleOutput *> ruleOutputList;

public:
	Rule(string id, uint32_t addr, string name, string type, unsigned char repeater);
	Rule(string id, uint32_t addr, string name, string type, unsigned char repeater, int startTime, int endTime);
	~Rule();

	void AddRuleInput(RuleInput *ruleInput);
	void AddRuleOutput(RuleOutput *ruleOutput);
	void DelAllRuleInput();
	void DelAllRuleOutput();
	void Check();
	void RunOutput();

	void UpdateData(string data);
	string GetData();
	bool GetStatus();
	void SetStatus(bool enable);
};
