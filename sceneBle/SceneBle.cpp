#include "SceneBle.h"
#include <thread>
#include "Log.h"
#include "BleProtocol.h"

DeviceInSceneBle::DeviceInSceneBle(Device *device, Json::Value data)
{
	this->device = device;
	this->data = data;
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

string SceneBle::GetUUId()
{
	return sceneBleUUId;
}

int SceneBle::GetPositionDevice(Device *device)
{
	uint32_t deviceAddr = device->GetAddr();
	for (int i = 0; i < deviceList.size(); i++)
	{
		if (deviceAddr == deviceList[i]->device->GetAddr())
		{
			return i;
		}
	}
	return -1;
}

bool SceneBle::AddDevice(Device *device, Json::Value data, int modeRGB, bool addOnlyDB)
{
	if (addOnlyDB == false)
	{
		if (bleProtocol->SetSceneLights(device->GetAddr(), id, modeRGB))
		{
			DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, data);
			deviceList.push_back(deviceInSceneBle);
			return true;
		}
	}
	else
	{
		DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, data);
		deviceList.push_back(deviceInSceneBle);
		return true;
	}
	return false;
}

bool SceneBle::DelDevice(Device *device)
{
	if (bleProtocol->DelSceneLights(device->GetAddr(), id))
	{
		int deviceIndex = GetPositionDevice(device);
		if (deviceIndex > -1)
		{
			deviceList.erase(deviceList.begin() + deviceIndex);
		}
		return true;
	}
	return false;
}

void SceneBle::Do(int id)
{
	bleProtocol->CallScene(0xff, id, 10, true, 1);
}
