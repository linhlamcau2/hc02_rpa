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
    int id;

public:
    vector<DeviceInRoom *> deviceList;
    vector<string> dataConfig;

    Room(string roomUUId, int id);
    string GetUUId();
    int GetId();
    int GetPositionDevice(Device *device);
    bool AddDevice(Device *device, bool sendBle);
    bool DelDevcie(Device *device);

    int DataConfigAdd(string data);
    int DataConfigDel(string data);
};