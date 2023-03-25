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
	return -1;
}

/**
 * Id room là id group tất cả thiết bị trong phòng
 * Lấy unicast room là unicast group tất cả thiết bị
 */
int Room::AddDevice(Device *device, bool sendBle)
{
	if (!device)
	{
		return CODE_ERROR;
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
						return CODE_OK;
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
					return CODE_OK;
				}
			}
		}
	}
	return CODE_ERROR;
}

int Room::AddDevice2(Device *device, bool sendBle)
{
	LOGW("AddDevice2");
	if (!device)
	{
		return CODE_ERROR;
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
						return CODE_OK;
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
					return CODE_OK;
				}
			}
		}
	}
	return CODE_ERROR;
}

int Room::DelDevice(Device *device)
{
	if (device->GetProtocol() == BLE_DEVICE)
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

int Room::DelDevice2(Device *device)
{
	LOGW("DelDevice2");
	return CODE_ERROR;
}

string Room::GetDataConfig()
{
	return dataConfig;
}

void Room::SetDataConfig(string dataConfig)
{
	this->dataConfig = dataConfig;
}
