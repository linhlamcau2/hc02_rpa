#pragma once

#include <vector>
#include <functional>

using namespace std;

typedef void (*ActionCallbackFuncType1)(string &topic, string &payload);
typedef function<void(string &topic, string &payload)> ActionCallbackFuncType2;

class ActionCallback
{
private:
	int type;
	string topic;

public:
	ActionCallbackFuncType1 actionCallbackFuncType1;
	ActionCallbackFuncType2 actionCallbackFuncType2;
	ActionCallback(ActionCallbackFuncType1 actionCallbackFuncType1, string topic);
	ActionCallback(ActionCallbackFuncType2 actionCallbackFuncType2, string topic);
	int getType();
	void setTopic(string topic);
	string getTopic();
};