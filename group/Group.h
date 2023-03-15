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
	int numberOfBleDevice;
	int numberOfZigbeeDevice;

	Json::Value dataValue;

public:
	vector<DeviceInGroup *> deviceList;

public:
	Group(string id, uint32_t addr, string name);

	int GetPositionDevice(Device *device);

	bool AddDevice(Device *device, int epId, bool sendBle);
	bool DelDevice(Device *device, int epId);

	bool Do(Json::Value &dataValue);
	bool DoV2(Json::Value &dataValue);
	void DoBle();
	void DoBleV2();
	void DoZigbee();
};
