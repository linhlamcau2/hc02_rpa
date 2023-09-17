#include "SceneBle.h"
#include <thread>
#include "Log.h"
#include "BleProtocol.h"
#include "BleDefine.h"

DeviceInSceneBle::DeviceInSceneBle(Device *device, Json::Value data)
{
	this->device = device;
	this->data = data;
}

SceneBle::SceneBle(string id, uint32_t addr, string name) : Object(id, addr, name)
{
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

SceneBle::~SceneBle()
{
	mtx.lock();
	deviceList.clear();
	mtx.unlock();
}

int SceneBle::GetPositionDevice(Device *device)
{
	uint32_t deviceAddr = device->GetAddr();
	mtx.lock();
	for (uint32_t i = 0; i < deviceList.size(); i++)
	{
		if (deviceAddr == deviceList[i]->device->GetAddr())
		{
			mtx.unlock();
			return i;
		}
	}
	mtx.unlock();
	return CODE_ERROR;
}

int SceneBle::AddDevice(Device *device, Json::Value data, bool addOnlyDB)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (addOnlyDB)
	{
		DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, data);
		deviceList.push_back(deviceInSceneBle);
		return CODE_OK;
	}
	else
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
			mtx.lock();
			deviceList.push_back(deviceInSceneBle);
			mtx.unlock();
			return CODE_OK;
		}
	}
#else
	if (addOnlyDB)
	{
		DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, data);
		mtx.lock();
		deviceList.push_back(deviceInSceneBle);
		mtx.unlock();
		return CODE_OK;
	}
	else
	{
		int modeRGB = 0;
		for (Json::ArrayIndex i = 0; i < data.size(); i++)
		{
			Json::Value property = data[i];
			if (property.isMember("ID") && property["ID"].isInt() && property.isMember("VALUE") && property["VALUE"].isInt())
			{
				if (property["ID"].asInt() == BLE_ATTRIBUTE_SCENE_RGB)
				{
					modeRGB = property["VALUE"].asInt();
					break;
				}
			}
		}
		if (bleProtocol->SetSceneBle(device->GetAddr(), addr, modeRGB) == 0)
		{
			DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, data);
			mtx.lock();
			deviceList.push_back(deviceInSceneBle);
			mtx.unlock();
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}

int SceneBle::DelDevice(Device *device)
{
	if (bleProtocol->DelSceneBle(device->GetAddr(), addr) == 0)
	{
		int deviceIndex = GetPositionDevice(device);
		if (deviceIndex > -1)
		{
			mtx.lock();
			deviceList.erase(deviceList.begin() + deviceIndex);
			mtx.unlock();
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

int SceneBle::Do()
{
	return bleProtocol->CallScene(0xffff, addr, TRANSITION_DEFAULT, true);
}
