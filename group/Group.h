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
	Json::Value dataValue;
	mutex mtx;

public:
	vector<DeviceInGroup *> deviceList;

public:
	Group(string id, uint32_t addr, string name);
	~Group();

	int GetPositionDevice(Device *device, int epid);

	int AddDevice(Device *device, int epId, bool sendBle, bool isCheckElement = true);
	int DelDevice(Device *device, int epId, bool isCheckElement = true);

	int Do(Json::Value &dataValue, bool ack);
	int DoV2(Json::Value &dataValue);
	void DoBle(bool ack);
	void DoBleV2();
	void DoZigbee();
};
