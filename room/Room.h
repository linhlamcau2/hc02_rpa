#pragma once

#include <string>
#include <vector>
#include <json.h>
#include "Device.h"

using namespace std;

class DeviceInRoom
{
public:
	Device *device;

	DeviceInRoom(Device *device);
};

class Room
{
private:
    string roomUUId;
public:
    vector<DeviceInRoom *> deviceList;

    Room(string roomUUId);
    string GetUUId();
    int GetPositionDevice(Device *device);
    bool AddDevice(Device *device);
    bool DelDevcie(Device* device);
};