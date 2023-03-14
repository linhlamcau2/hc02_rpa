#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "json.h"
#include "Device.h"

using namespace std;

class DeviceInSceneBle
{
public:
	Device *device;
	Json::Value data;
	DeviceInSceneBle(Device *device, Json::Value data);
};

class SceneBle
{
private:
	int id;
	string name;
	string sceneBleUUId;

public:
	vector<DeviceInSceneBle *> deviceList;
	SceneBle(string sceneBleUUId, int id, string name);
	int GetId();
	string GetUUId();
	void SetUuid(string uuid);
	string GetName();
	void SetName(string name);
	int GetPositionDevice(Device *device);
	bool AddDevice(Device *device, Json::Value data, int modeRGB, bool addOnlyDB);
	bool AddDeviceV2(Device *device, Json::Value data, bool addOnlyDB);
	bool DelDevice(Device *device);
	bool Do();
};
