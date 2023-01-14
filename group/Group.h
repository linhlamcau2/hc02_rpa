#pragma once

#include <string>
#include <vector>
#include <json.h>
#include "Device.h"

using namespace std;

class DeviceInGroup
{
public:
	Device *device;
	int epId;

	DeviceInGroup(Device *device, int epId);
};

class Group
{
private:
	int id;
	string name;
	string groupUUId;

	int numberOfBleDevice;
	int numberOfZigbeeDevice;

	Json::Value dataValue;
	
public:
	vector<DeviceInGroup *> deviceList;

public:
	Group(string groupUUId, int id, string name);

	int GetId();
	void SetName(string name);
	string GetName();
	string GetUUId();
	int GetPositionDevice(Device *device);
	
	bool AddDevice(Device *device, int epId);
	void DelDevice(Device *device, int epId);

	bool Do(Json::Value &dataValue);
	bool Do(int id, int value);
	void DoBle(Json::Value *dataValue);
	void DoZigbee(Json::Value *dataValue);
};
