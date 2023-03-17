#pragma once

#include <iostream>
#include <string>
#include <vector>
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
public:
	vector<DeviceInSceneBle *> deviceList;
	SceneBle(string id, uint32_t addr, string name);
	
	int GetPositionDevice(Device *device);
	int AddDevice(Device *device, Json::Value data, int modeRGB, bool addOnlyDB);
	int AddDeviceV2(Device *device, Json::Value data, bool addOnlyDB);
	int DelDevice(Device *device);
	int Do();
};
