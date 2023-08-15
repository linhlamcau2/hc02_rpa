#pragma once

#include <string>
#include <vector>
#include <mutex>
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
	mutex mtx;

public:
	vector<DeviceInGroup *> deviceList;

public:
	Group(string id, uint16_t addr, string name);
	~Group();

	int GetPositionDevice(Device *device);

	int AddDevice(Device *device, int epId, bool sendBle);
	int DelDevice(Device *device, int epId);

	int Do(Json::Value &dataValue, bool ack = true);
	// int DoZigbee();
};
