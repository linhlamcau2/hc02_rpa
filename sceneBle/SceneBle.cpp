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

void SceneBle::SetUuid(string uuid)
{
	this->sceneBleUUId = uuid;
}

string SceneBle::GetName()
{
	return name;
}

void SceneBle::SetName(string name)
{
	this->name = name;
}

int SceneBle::GetPositionDevice(Device *device)
{
	uint32_t deviceAddr = device->GetAddr();
	for (uint32_t i = 0; i < deviceList.size(); i++)
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
		if (bleProtocol->SetSceneBle(device->GetAddr(), id, modeRGB) == 0)
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

bool SceneBle::AddDeviceV2(Device *device, Json::Value data, bool addOnlyDB)
{
	if (addOnlyDB == false)
	{
		if (data.isObject() &&
				data.isMember(KEY_ATTRIBUTE_MODE_RGB) && data[KEY_ATTRIBUTE_MODE_RGB].isInt())
		{
			int modeRGB = data[KEY_ATTRIBUTE_MODE_RGB].asInt();
			if (bleProtocol->SetSceneBle(device->GetAddr(), id, modeRGB) == 0)
			{
				DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, data);
				deviceList.push_back(deviceInSceneBle);
				return true;
			}
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
	if (bleProtocol->DelSceneBle(device->GetAddr(), id) == 0)
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

bool SceneBle::Do()
{
	return bleProtocol->CallScene(0xffff, id, 10, true, 1);
}
