#include "Room.h"
#include <Log.h>
#include "BleProtocol.h"

DeviceInRoom::DeviceInRoom(Device *device)
{
    this->device = device;
}

Room::Room(string roomUUId, int id)
{
    this->roomUUId = roomUUId;
    this->id = id;
}

string Room::GetUUId()
{
    return roomUUId;
}
int Room::GetId()
{
    return id;
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
bool Room::AddDevice(Device *device, bool sendBle)
{
    if (!device)
    {
        return false;
    }
    if (device->GetProtocol() == BLE_DEVICE)
    {
        DeviceInRoom *devcieInRoom = new DeviceInRoom(device);

        if (sendBle)
        {
            if (bleProtocol->SetGroup(device->GetAddr(), id + 49152) == 0)
            {
                if (devcieInRoom)
                {
                    if (GetPositionDevice(device) == -1)
                    {
                        deviceList.push_back(devcieInRoom);
                        return true;
                    }
                }
            }
            else
            {
                LOGW("Add ble device %s to smart home room %s error", device->GetId().c_str(), roomUUId.c_str());
            }
        }
        else
        {
            if (devcieInRoom)
            {
                if (GetPositionDevice(device) == -1)
                {
                    deviceList.push_back(devcieInRoom);
                    return true;
                }
            }
        }
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

int Room::DataConfigAdd(string data)
{
    for (int i = 0; i < dataConfig.size(); i++)
    {
        if (dataConfig[i] == data)
        {
            return -1;
        }
    }
    dataConfig.push_back(data);
    return 0;
}

int Room::DataConfigDel(string data)
{
    for (int i = 0; i < dataConfig.size(); i++)
    {
        if (dataConfig[i] == data)
        {
            dataConfig.erase(dataConfig.begin() + i);
            return 0;
        }
    }
    return -1;
}