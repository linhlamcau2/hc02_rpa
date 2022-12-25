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
	int epId;

	DeviceInSceneBle(Device *device, int epId);
};

class SceneBle
{
    private:
        int id;
        string name;
        string sceneBleUUId;
        vector<SceneOutput *> sceneOutputList;

    public:
        SceneBle();
        SceneBle(string sceneBleUUId, int id, string name);
        int Init(string sceneBleUUId, int id, string name);
        vector<DeviceInSceneBle *> deviceList;
        int GetId();
        string GetUUId();
        int GetPositionDevice(Device *device);
        bool AddDevice(Device *device, int epId);
        bool DelDevice(Device *device, int epId);
        void Do(int id);
};