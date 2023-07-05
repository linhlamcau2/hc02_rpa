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

public:
	vector<SceneBle *> sceneBleList;
	vector<Group *> groupList;

	Room(string id, uint32_t addr, string name);
	~Room();

	int GetPositionDevice(Device *device);
	int GetPositionGroup(Group *group);
	int GetPositionSceneBle(SceneBle *sceneBle);
	int AddDevice(Device *device, int epId, bool sendBle);
	int AddGroup(Group *group, bool isAddGateway, bool isAddDatabase);
	int DelGroup(Group *group);
	int AddSceneBle(SceneBle *sceneBle, bool isAddGateway, bool isAddDatabase);
	int DelSceneBle(SceneBle *sceneBle);

	string GetDataConfig();
	void SetDataConfig(string dataConfig);
};
