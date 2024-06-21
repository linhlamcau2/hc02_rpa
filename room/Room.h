#pragma once

#include <string>
#include <vector>
#include <mutex>
#include "json.h"
#include "Object.h"
#include "Device.h"
#include "Group.h"
#include "SceneBle.h"

using namespace std;

class Room : public Group
{
private:
	string dataConfig;
	mutex mtxGroup;
	mutex mtxScene;
	mutex mtxRule;

public:
	vector<SceneBle *> sceneBleList;
	vector<Group *> groupList;
	vector<Rule *> ruleList;

	Room(string id, uint16_t addr, string name);
	~Room();

	int GetPositionGroup(Group *group);
	int GetPositionSceneBle(SceneBle *sceneBle);
	int GetPositionRule(Rule *rule);

	int AddDeviceOneMessage(Device *device, bool sendBle, bool addDb);
	int DelDeviceOneMessage(Device *device, bool sendBle, bool delDb);
	int AddDevice(Device *device, bool sendBle, bool addDb);
	int DelDevice(Device *device, bool sendBle, bool delDb);
	int AddGroup(Group *group, bool isAddGateway, bool isAddDatabase);
	int DelGroup(Group *group, bool delDb);
	int AddSceneBle(SceneBle *sceneBle, bool isAddGateway, bool isAddDatabase);
	int DelSceneBle(SceneBle *sceneBle, bool delDb);
	int AddRule(Rule *rule, bool isAddGateway, bool isAddDatabase);
	int DelRule(Rule *rule, bool delDb);

	string GetDataConfig();
	void SetDataConfig(string dataConfig);
};
