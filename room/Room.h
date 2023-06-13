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

class DeviceInRoom
{
public:
	Device *device;
	DeviceInRoom(Device *device);
};

class Room : public Object
{
private:
	string dataConfig;
	mutex mtxDev;
	mutex mtxGroup;
	mutex mtxScene;

public:
	vector<DeviceInRoom *> deviceList;
	vector<SceneBle *> sceneBleList;
	vector<Group *> groupList;

	Room(string id, uint32_t addr, string name);
	~Room();

	int GetPositionDevice(Device *device);
	int GetPositionGroup(Group *group);
	int GetPositionSceneBle(SceneBle *sceneBle);
	int AddDevice(Device *device, bool sendBle);
	int AddDevice2(Device *device, bool sendBle);
	int DelDevice(Device *device);
	int DelDevice2(Device *device);
	int AddGroup(Group *group, bool isAddGateway, bool isAddDatabase);	
	int DelGroup(Group *group);
	int AddSceneBle(SceneBle *sceneBle, bool isAddGateway, bool isAddDatabase);	
	int DelSceneBle(SceneBle *sceneBle);

	string GetDataConfig();
	void SetDataConfig(string dataConfig);
};
