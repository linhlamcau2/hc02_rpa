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

SceneBle::~SceneBle()
{
	deviceList.clear();
}

bool SceneBle::GetIsFavorite()
{
	return this->isFavorite;
}

bool SceneBle::SetIsFavorite(bool isFavorite)
{
	this->isFavorite = isFavorite;
	return this->isFavorite;
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

int SceneBle::AddDevice(Device *device, Json::Value data, int modeRGB, bool addOnlyDB)
{
	if (addOnlyDB == false)
	{
		if (bleProtocol->SetSceneBle(device->GetAddr(), addr, modeRGB) == 0)
		{
			DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, data);
			deviceList.push_back(deviceInSceneBle);
			return CODE_OK;
		}
	}
	else
	{
		DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, data);
		deviceList.push_back(deviceInSceneBle);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int SceneBle::AddDeviceV2(Device *device, Json::Value data, bool addOnlyDB)
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
			return CODE_OK;
		}
	}
	else
	{
		DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, data);
		deviceList.push_back(deviceInSceneBle);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int SceneBle::DelDevice(Device *device)
{
	if (bleProtocol->DelSceneBle(device->GetAddr(), addr) == 0)
	{
		int deviceIndex = GetPositionDevice(device);
		if (deviceIndex > -1)
		{
			deviceList.erase(deviceList.begin() + deviceIndex);
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

int SceneBle::Do()
{
	return bleProtocol->CallScene(0xffff, addr, 10, true, 1);
}
