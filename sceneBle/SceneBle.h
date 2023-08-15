#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include "json.h"
#include "Object.h"
#include "Device.h"

using namespace std;

class DeviceInSceneBle
{
public:
	Device *device;
	Json::Value data;
	DeviceInSceneBle(Device *device, Json::Value data);
};

class SceneBle : public Object
{
private:
	bool isFavorite;
	mutex mtx;

public:
	vector<DeviceInSceneBle *> deviceList;
	SceneBle(string id, uint16_t addr, string name);
	~SceneBle();
	bool GetIsFavorite();
	bool SetIsFavorite(bool isFavorite);
	int GetPositionDevice(Device *device);
	int AddDevice(Device *device, Json::Value data, bool addOnlyDB);
	int DelDevice(Device *device);
	int Do();
};
