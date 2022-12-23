#include "Group.h"
#include <thread>
#include <Log.h>
#include "ZigbeeProtocol.h"
#include "BleProtocol.h"

DeviceInGroup::DeviceInGroup(Device *device, int epId)
{
	this->device = device;
	this->epId = epId;
}

Group::Group(string groupUUId, int id, string name)
{
	this->groupUUId = groupUUId;
	this->id = id;
	this->name = name;
	this->numberOfBleDevice = 0;
	this->numberOfZigbeeDevice = 0;
}

int Group::GetId()
{
	return id;
}

void Group::SetName(string name)
{
	this->name = name;
}

string Group::GetName()
{
	return name;
}

int Group::GetPositionDevice(Device *device)
{
	int deviceAddr = device->GetAddr();
	for (int i=0; i<deviceList.size(); i++)
	{
		if(deviceAddr = deviceList[i]->device->GetAddr())
		{
			return i;
		}
	}
	return -1;
}

/**
 * @brief Add device function
 * 
 * @param device ID Device
 * @param epId Element ID
 * @return true success
 * @return false fail
 */
bool Group::AddDevice(Device *device, int epId)
{
	// TODO: Check exsit
	// if (std::find(deviceList.begin(), deviceList.end(), device) != deviceList.end())
	// 	return false;
	if (!device)
		return false;

	if (device->GetProtocol() == BLE_DEVICE)
	{
		if (bleProtocol->AddDev2Group(device->GetAddr(), epId, id) == 0)
		{
			DeviceInGroup *deviceInGroup = new DeviceInGroup(device, epId);
			if (deviceInGroup)
			{
				deviceList.push_back(deviceInGroup);
				numberOfBleDevice++;
			}
			return true;
		}
		else
		{
			LOGW("Add Ble device %s to group %d error", device->GetId().c_str(), id);
		}
	}

#ifdef CONFIG_ENABLE_ZIGBEE
	if (device->GetProtocol() == ZIGBEE_DEVICE)
	{
		if (zigbeeProtocol->AddGroup(id, device->GetAddr(), epId))
		{
			DeviceInGroup *deviceInGroup = new DeviceInGroup(device, epId);
			if (deviceInGroup)
			{
				deviceList.push_back(deviceInGroup);
				numberOfZigbeeDevice++;
			}
			return true;
		}
		else
		{
			LOGW("Add Zigbee device %s to group %d error", device->GetId().c_str(), id);
		}
	}
#endif
	return false;
}

void Group::DelDevice(Device *device, int epId)
{
	if (device->GetProtocol() == BLE_DEVICE)
	{
		if (bleProtocol->DelDev2Group(device->GetAddr(), epId, id))
		{
			int deviceIndex = GetPositionDevice(device);
			if (deviceIndex > -1)
			{
				deviceList.erase(deviceList.begin()+deviceIndex);
			}
		}   
	}

#ifdef CONFIG_ENABLE_ZIGBEE
	if (device->GetProtocol() == ZIGBEE_DEVICE)
	{
		// TODO: remove from group
		numberOfZigbeeDevice--;
		// zigbeeProtocol->AddGroup(id, device->GetAddr(), epId);
	}
#endif
	// TODO: remove from list
	// if (device)
	// 	deviceList.erase(remove(deviceList.begin(), deviceList.end(), device), deviceList.end());
}

bool Group::Do(Json::Value &dataValue)
{
	this->dataValue = dataValue;

	auto doBleBind = bind(&Group::DoBle, this, placeholders::_1);
	thread doBleThread(doBleBind, &this->dataValue);
	doBleThread.detach();

#ifdef CONFIG_ENABLE_ZIGBEE
	auto doZigbeeBind = bind(&Group::DoZigbee, this, placeholders::_1);
	thread doZigbeeThread(doZigbeeBind, &this->dataValue);
	doZigbeeThread.detach();
#endif

	return true;
}

bool Group::Do(int id, int value)
{
	LOGD("Do group");
	return true;
}

void Group::DoBle(Json::Value *dataValue)
{
	if (numberOfBleDevice)
	{
		if (dataValue->isMember("method") && (*dataValue)["method"].isString())
		{
			string method = (*dataValue)["method"].asString();
			if (method == "TurnOn")
			{
				bleProtocol->TurnOnOff(0xC000 + id, 0);
			}
			else if (method == "TurnOff")
			{
				bleProtocol->TurnOnOff(0xC000 + id, 1);
			}
			else if (method == "Toggle")
			{
				bleProtocol->TurnOnOff(0xC000 + id, 2);
			}
			else
			{
				LOGW("Ble Group not handle method %s", method.c_str());
			}
		}
	}
}

#ifdef CONFIG_ENABLE_ZIGBEE
void Group::DoZigbee(Json::Value *dataValue)
{
	if (numberOfZigbeeDevice)
	{
		if (dataValue->isMember("method") && (*dataValue)["method"].isString())
		{
			string method = (*dataValue)["method"].asString();
			if (method == "TurnOn")
			{
				zigbeeProtocol->ZCLOnoffGroup(id, 0);
			}
			else if (method == "TurnOff")
			{
				zigbeeProtocol->ZCLOnoffGroup(id, 1);
			}
			else if (method == "Toggle")
			{
				zigbeeProtocol->ZCLOnoffGroup(id, 2);
			}
			else
			{
				LOGW("Zigbee Group not handle method %s", method.c_str());
			}
		}
	}
}
#endif
