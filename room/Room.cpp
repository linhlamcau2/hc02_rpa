#include "Room.h"
#include <Log.h>
#include "BleProtocol.h"

DeviceInRoom::DeviceInRoom(Device *device)
{
    this->device = device;
}

Room::Room(string roomUUId)
{
    this->roomUUId = roomUUId;
}

string Room::GetUUId()
{
    return roomUUId;
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

bool Room::AddDevice(Device *device)
{
    if (device->GetProtocol() == BLE_DEVICE)
    {
        DeviceInRoom *devcieInRoom = new DeviceInRoom(device);
        if (GetPositionDevice(device) == -1)
        {
            deviceList.push_back(devcieInRoom);
        }
        return true;
    }
    return false;
}

bool Room::DelDevcie(Device *device)
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