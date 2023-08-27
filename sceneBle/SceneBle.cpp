#include "SceneBle.h"
#include "Log.h"
#include "BleProtocol.h"
#include "BleDefine.h"
#include "Db.h"

DeviceInSceneBle::DeviceInSceneBle(Device *device, Json::Value data)
{
	this->device = device;
	this->data = data;
}

SceneBle::SceneBle(string id, uint16_t addr, string name) : Object(id, addr, name)
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

int SceneBle::AddDevice(Device *device, Json::Value data, bool sendBle, bool addDb)
{
	if (!device)
		return CODE_ERROR;

	DeviceInSceneBle *deviceInSceneBle = new DeviceInSceneBle(device, data);
	if (!deviceInSceneBle)
		return CODE_ERROR;
	if (GetPositionDevice(device) == CODE_ERROR)
	{
		if (!sendBle)
		{
			mtx.lock();
			deviceList.push_back(deviceInSceneBle);
			mtx.unlock();

			if (addDb)
				database->DeviceInSceneBleAdd(this, device, data.toString());
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
			if (bleProtocol->SetSceneBle(device->GetAddr(), addr + ID_START, modeRGB) == CODE_OK)
			{
				mtx.lock();
				deviceList.push_back(deviceInSceneBle);
				mtx.unlock();

				if (addDb)
					database->DeviceInSceneBleAdd(this, device, data.toString());
				return CODE_OK;
			}
		}
	}
	else
	{
		LOGW("Device %s is exist in sceneble", device->GetId().c_str());
		return CODE_OK;
	}
	return CODE_ERROR;
}

int SceneBle::DelDevice(Device *device, bool sendBle, bool delDb)
{
	if (!device)
		return CODE_ERROR;

	if (delDb)
		database->DeviceInSceneBleDel(this, device);
	if (sendBle)
	{
		if (bleProtocol->DelSceneBle(device->GetAddr(), addr + ID_START) == CODE_OK)
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
	}
	else
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
	return bleProtocol->CallScene(0xffff, addr + ID_START, 10, true, 1);
}
