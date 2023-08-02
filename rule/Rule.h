#pragma once

#include <string>
#include <vector>
#include "Object.h"
#include "RuleInput.h"
#include "RuleOutput.h"
#include "json.h"
#include <unistd.h>

using namespace std;

typedef enum
{
	RULE_TYPE_TAP_TO_RUN = -2,
	RULE_TYPE_TIME = -1,
	RULE_TYPE_OR = 0,
	RULE_TYPE_AND,
	RULE_TYPE_TIME_OR,
	RULE_TYPE_TIME_AND
} RuleType;

typedef enum
{
	RULE_MODE_ALL_DAY = 0xFE,
	RULE_MODE_WORK_DAY = 0xF1,
	RULE_MODE_WEEKEND_DAY = 0x06,
} RuleMode;

class Rule : public Object
{
private:
	Json::Value ruleData;
	RuleType type;
	string name;

	unsigned char repeater;
	bool fullDay;
	int startTime;
	int endTime;

	bool isAvailable;
	bool isEnable;
	int timerRegisterIndex;

	vector<RuleInput *> ruleInputList;
	vector<RuleOutput *> ruleOutputList;

public:
	Rule(string id, RuleType type, unsigned char repeater, string name, uint32_t addr, Json::Value &ruleData);
	Rule(string id, RuleType type, unsigned char repeater, string name, uint32_t addr, int startTime, int endTime, Json::Value &ruleData);
	~Rule();

	Json::Value GetRuleData();
	RuleType GetType();
	void AddRuleInput(RuleInput *ruleInput);
	void AddRuleOutput(RuleOutput *ruleOutput);
	void DelAllRuleInput();
	void DelAllRuleOutput();
	void Check();
	void RunOutput();

	void UpdateData(Json::Value &ruleData);
	bool GetStatus();
	void SetStatus(bool enable);
};
