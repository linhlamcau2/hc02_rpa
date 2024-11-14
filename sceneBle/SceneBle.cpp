#include "SceneBle.h"
#include "Log.h"
#include "BleProtocol.h"
#include "Db.h"

DeviceInSceneBle::DeviceInSceneBle(Device *device, Json::Value data)
{
	this->device = device;
	this->data = data;
}

SceneBle::SceneBle(string id, uint16_t addr, string name) : Object(id, addr, name)
{
	this->isFavorite = false;
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
	int positionDevice = GetPositionDevice(device);
	if (!sendBle)
	{
		mtx.lock();
		if (positionDevice != CODE_ERROR)
		{
			deviceList.erase(deviceList.begin() + positionDevice);
		}
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
		int indexType = device->GetType() / 1000;
		if (indexType != 22 && indexType != 24) // sceneble for light
		{
			if (bleProtocol->SetSceneBle(device->GetAddr(), addr, modeRGB) == CODE_OK)
			{
				mtx.lock();
				if (positionDevice != CODE_ERROR)
				{
					deviceList.erase(deviceList.begin() + positionDevice);
				}
				deviceList.push_back(deviceInSceneBle);
				mtx.unlock();

				if (addDb)
					database->DeviceInSceneBleAdd(this, device, data.toString());
				return CODE_OK;
			}
		}
		else // sceneble for switch touch, electrical
		{
			bool isSuccess = true;
			for (int i = 0; i < device->GetNumElement(); i++)
			{
				if (data.isMember(KEY_ATTRIBUTE_ONOFF) && data[KEY_ATTRIBUTE_ONOFF].isInt())
				{
					if (bleProtocol->SetSceneBle(device->GetAddr() + i, addr, 0) != CODE_OK)
						isSuccess = false;
				}

				if (data.isMember(KEY_ATTRIBUTE_BUTTON + ((i) ? to_string(i + 1) : "")))
				{
					if (bleProtocol->SetSceneBle(device->GetAddr() + i, addr, 0) != CODE_OK)
						isSuccess = false;
				}
			}
			mtx.lock();
			if (positionDevice != CODE_ERROR)
			{
				deviceList.erase(deviceList.begin() + positionDevice);
			}
			deviceList.push_back(deviceInSceneBle);
			mtx.unlock();

			if (addDb)
				database->DeviceInSceneBleAdd(this, device, data.toString());

			if (isSuccess)
				return CODE_OK;
			else
				return CODE_ERROR;
		}
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
		int indexType = device->GetType() / 1000;
		if (indexType != 22 && indexType != 24) // del scene ble for light
		{
			if (bleProtocol->DelSceneBle(device->GetAddr(), addr) == CODE_OK)
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
		else // del sceneble for switch touch, electrical
		{
			vector<DeviceInSceneBle *> tempDeviceList = deviceList;
			for (auto &devInSceneBle : tempDeviceList)
			{
				if (devInSceneBle->device->GetId() == device->GetId())
				{
					for (int i = 0; i < devInSceneBle->device->GetNumElement(); i++)
					{
						if (devInSceneBle->data.isMember(KEY_ATTRIBUTE_ONOFF) && devInSceneBle->data[KEY_ATTRIBUTE_ONOFF].isInt())
						{
							bleProtocol->DelSceneBle(device->GetAddr() + i, addr);
						}
						if (devInSceneBle->data.isMember(KEY_ATTRIBUTE_BUTTON + ((i) ? to_string(i + 1) : "")))
						{
							bleProtocol->DelSceneBle(device->GetAddr() + i, addr);
						}
					}
					int deviceIndex = GetPositionDevice(devInSceneBle->device);
					if (deviceIndex > -1)
					{
						mtx.lock();
						deviceList.erase(deviceList.begin() + deviceIndex);
						mtx.unlock();
					}
					return CODE_OK;
				}
			}
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

int SceneBle::Do(bool ack)
{
	return bleProtocol->CallScene(0xffff, addr, TRANSITION_DEFAULT, ack);
}
