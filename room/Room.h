#pragma once

#include <string>
#include <vector>
#include "json.h"
#include "Object.h"
#include "Device.h"

using namespace std;

class DeviceInRoom
{
public:
	Device *device;
	DeviceInRoom(Device *device);
};

class Room : public Object
{
private:
	string dataConfig;

public:
	vector<DeviceInRoom *> deviceList;
	Room(string id, uint32_t addr, string name);

	int GetPositionDevice(Device *device);
	bool AddDevice(Device *device, bool sendBle);
	bool AddDevice2(Device *device, bool sendBle);
	bool DelDevcie(Device *device);

	string GetDataConfig();
	void SetDataConfig(string dataConfig);
};