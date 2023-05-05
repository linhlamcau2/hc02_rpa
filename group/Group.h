#pragma once

#include <string>
#include <vector>
#include "json.h"
#include "Object.h"
#include "Device.h"

using namespace std;

class DeviceInGroup
{
public:
	Device *device;
	int epId;
	DeviceInGroup(Device *device, int epId);
};

class Group : public Object
{
private:
	Json::Value dataValue;

public:
	vector<DeviceInGroup *> deviceList;

public:
	Group(string id, uint32_t addr, string name);

	int GetPositionDevice(Device *device);

	int AddDevice(Device *device, int epId, bool sendBle);
	int DelDevice(Device *device, int epId);

	int Do(Json::Value &dataValue);
	int DoV2(Json::Value &dataValue);
	void DoBle();
	void DoBleV2();
	void DoZigbee();
};
