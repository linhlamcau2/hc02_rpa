#include "SceneBle.h"
#include <thread>
#include "../log/Log.h"
#include "../protocol/BleProtocol.h"

DeviceInSceneBle::DeviceInSceneBle(Device *device, int epId)
{
    this->device = device;
	this->epId = epId;
}

int SceneBle::Init(string sceneBleUUId, int id, string name)
{
    this->id = id;
    this->sceneBleUUId = sceneBleUUId;
    this->name = name;
}

SceneBle::SceneBle(){
}


SceneBle::SceneBle(string sceneBleUUId, int id, string name)
{
    this->id = id;
    this->sceneBleUUId = sceneBleUUId;
    this->name = name;
}

int SceneBle::GetId()
{
    return id;
}

string SceneBle::GetUUId(){
    return sceneBleUUId;
}

int SceneBle::GetPositionDevice(Device *device)
{
	int deviceAddr = device->GetAddr();
	for (int i=0; i<deviceList.size(); i++)
	{
		if(deviceAddr = deviceList[i]->device->GetAddr())
		{
			return i;
		}
	}
	return -1;
}

bool SceneBle::AddDevice(Device *device, int epId)
{
    if (bleProtocol->SetSceneLights(device->GetAddr(), id, epId))
    {
        DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, epId);
        deviceList.push_back(deviceInSceneBle);
    }
}

bool SceneBle::DelDevice(Device *device, int epId)
{
    if (bleProtocol->DelSceneLights(device->GetAddr(), id))
    {
        int deviceIndex = GetPositionDevice(device);
        if (deviceIndex > -1)
        {
            deviceList.erase(deviceList.begin()+deviceIndex);
        }
    }   
}

void SceneBle::Do(int id)
{
//    bleProtocol->CallScene(id, 10, true);
}
