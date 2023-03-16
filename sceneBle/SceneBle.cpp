#include "SceneBle.h"
#include <thread>
#include "Log.h"
#include "BleProtocol.h"

DeviceInSceneBle::DeviceInSceneBle(Device *device, Json::Value data)
{
	this->device = device;
	this->data = data;
}

SceneBle::SceneBle(string id, uint32_t addr, string name) : Object(id, addr, name)
{
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
	return CODE_ERROR;
}

bool SceneBle::AddDevice(Device *device, Json::Value data, int modeRGB, bool addOnlyDB)
{
	if (addOnlyDB == false)
	{
		if (bleProtocol->SetSceneBle(device->GetAddr(), addr, modeRGB) == 0)
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
		int modeRGB = 0;
		if (data.isObject() &&
				data.isMember(KEY_ATTRIBUTE_MODE_RGB) && data[KEY_ATTRIBUTE_MODE_RGB].isInt())
		{
			modeRGB = data[KEY_ATTRIBUTE_MODE_RGB].asInt();
		}
		if (bleProtocol->SetSceneBle(device->GetAddr(), addr, modeRGB) == 0)
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
	if (bleProtocol->DelSceneBle(device->GetAddr(), addr) == 0)
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
	return bleProtocol->CallScene(0xffff, addr, 10, true, 1);
}
