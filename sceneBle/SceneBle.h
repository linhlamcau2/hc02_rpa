#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "json.h"
#include "Device.h"

using namespace std;

class DeviceInSceneBle
{
public:
	Device *device;
    Json::Value data;
	DeviceInSceneBle(Device *device, Json::Value data);
};

class SceneBle
{
    private:
        int id;
        string name;
        string sceneBleUUId;
    public:
        vector<DeviceInSceneBle *> deviceList;
        SceneBle(string sceneBleUUId, int id, string name);
        int GetId();
        string GetUUId();
        string GetName();
        int GetPositionDevice(Device *device);
        bool AddDevice(Device *device, Json::Value data, int modeRGB, bool addOnlyDB);
        bool DelDevice(Device *device);
        void Do(int id);
};
