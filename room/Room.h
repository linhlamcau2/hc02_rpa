#pragma once

#include <string>
#include <vector>
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

public:
	vector<DeviceInRoom *> deviceList;
	vector<SceneBle *> sceneBleList;
	vector<Group *> groupList;
	vector<string> dataConfig;

	Room(string id, uint32_t addr, string name);

	int GetPositionDevice(Device *device);
	int AddDevice(Device *device, bool sendBle);
	int AddDevice2(Device *device, bool sendBle);
	int DelDevice(Device *device);
	int DelDevice2(Device *device);

	string GetDataConfig();
	void SetDataConfig(string dataConfig);
};
