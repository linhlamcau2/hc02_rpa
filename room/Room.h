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
public:
	vector<DeviceInRoom *> deviceList;
	vector<SceneBle *> sceneBleList;
	vector<Group *> groupList;
	vector<string> dataConfig;

	Room(string id, uint32_t addr, string name);

	int GetPositionDevice(Device *device);
	bool AddDevice(Device *device, bool sendBle);
	bool AddDevice2(Device *device, bool sendBle);
	bool DelDevice(Device *device);
	bool DelDevice2(Device *device);

	int DataConfigAdd(string data);
	int DataConfigDel(string data);
};