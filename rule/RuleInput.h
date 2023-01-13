#pragma once

using namespace std;

class Rule;
class RuleInput
{
protected:
	bool isAvailable;
	Rule *rule;

public:
	virtual ~RuleInput() {}
	virtual bool Check() { return isAvailable; }
};