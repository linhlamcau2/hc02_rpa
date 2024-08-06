#pragma once

#include <string>
#include <vector>
#include <mutex>
#include "json.h"
#include "Object.h"
#include "Device.h"

#define ID_START (0xC000)

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

	int GetPositionDevice(Device *device, int epid);

	int AddDevice(Device *device, int epId, bool sendBle, bool addDb);
	int DelDevice(Device *device, int epId, bool sendBle, bool delDB);

	int Do(Json::Value &dataValue, bool ack = true);
	// int DoZigbee();
};
