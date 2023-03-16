#include "Room.h"
#include "Log.h"
#include "BleProtocol.h"

DeviceInRoom::DeviceInRoom(Device *device)
{
	this->device = device;
}

Room::Room(string id, uint32_t addr, string name) : Object(id, addr, name)
{
}

int Room::GetPositionDevice(Device *device)
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

/**
 * Id room là id group tất cả thiết bị trong phòng
 * Lấy unicast room là unicast group tất cả thiết bị
 */
bool Room::AddDevice(Device *device, bool sendBle)
{
	if (!device)
	{
		return false;
	}
	if (device->GetProtocol() == BLE_DEVICE)
	{
		DeviceInRoom *deviceInRoom = new DeviceInRoom(device);

		if (sendBle)
		{
			if (bleProtocol->SetGroup(device->GetAddr(), addr + 49152) == 0)
			{
				if (deviceInRoom)
				{
					if (GetPositionDevice(device) == -1)
					{
						deviceList.push_back(deviceInRoom);
						return true;
					}
				}
			}
			else
			{
				LOGW("Add ble device %s to smart home room %s error", device->GetId().c_str(), id.c_str());
			}
		}
		else
		{
			if (deviceInRoom)
			{
				if (GetPositionDevice(device) == -1)
				{
					deviceList.push_back(deviceInRoom);
					return true;
				}
			}
		}
	}
	return false;
}

bool Room::AddDevice2(Device *device, bool sendBle)
{
	LOGW("AddDevice2");
	if (!device)
	{
		return false;
	}
	if (device->GetProtocol() == BLE_DEVICE)
	{
		DeviceInRoom *deviceInRoom = new DeviceInRoom(device);
		if (sendBle)
		{
			if (bleProtocol->AddDeviceToRoom(device->GetAddr(), addr) == 0)
			{
				if (deviceInRoom)
				{
					if (GetPositionDevice(device) == -1)
					{
						deviceList.push_back(deviceInRoom);
						return true;
					}
				}
			}
			else
			{
				LOGW("Add ble device %s to smart home room %s error", device->GetId().c_str(), id.c_str());
			}
		}
		else
		{
			if (deviceInRoom)
			{
				if (GetPositionDevice(device) == -1)
				{
					deviceList.push_back(deviceInRoom);
					return true;
				}
			}
		}
	}
	return false;
}

bool Room::DelDevice(Device *device)
{
	if (device->GetProtocol() == BLE_DEVICE)
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

bool Room::DelDevice2(Device *device)
{
	LOGW("DelDevice2");
	return false;
}

int Room::DataConfigAdd(string data)
{
	for (uint32_t i = 0; i < dataConfig.size(); i++)
	{
		if (dataConfig[i] == data)
		{
			return CODE_ERROR;
		}
	}
	dataConfig.push_back(data);
	return CODE_OK;
}

int Room::DataConfigDel(string data)
{
	for (uint32_t i = 0; i < dataConfig.size(); i++)
	{
		if (dataConfig[i] == data)
		{
			dataConfig.erase(dataConfig.begin() + i);
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
