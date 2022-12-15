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
	this->numberOfLoraDevice = 0;
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

#ifdef CONFIG_ENABLE_BLE
	if (device->GetProtocol() == BLE_DEVICE)
	{
		if (bleProtocol->AddGroup(id, device->GetAddr(), epId) == 0)
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
#endif
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
#ifdef CONFIG_ENABLE_BLE
	if (device->GetProtocol() == BLE_DEVICE)
	{
		// TODO: remove from group
		numberOfBleDevice--;
		// bleProtocol->AddGroup(id, device->GetAddr(), epId);
	}
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
	if (device->GetProtocol() == ZIGBEE_DEVICE)
	{
		// TODO: remove from group
		numberOfZigbeeDevice--;
		// zigbeeProtocol->AddGroup(id, device->GetAddr(), epId);
	}
#endif
#ifdef CONFIG_ENABLE_LORA
	if (device->GetProtocol() == LORA_DEVICE)
	{
		numberOfLoraDevice--;
	}
#endif
	// TODO: remove from list
	// if (device)
	// 	deviceList.erase(remove(deviceList.begin(), deviceList.end(), device), deviceList.end());
}

bool Group::Do(Json::Value &dataValue)
{
	this->dataValue = dataValue;

#ifdef CONFIG_ENABLE_BLE
	auto doBleBind = bind(&Group::DoBle, this, placeholders::_1);
	thread doBleThread(doBleBind, &this->dataValue);
	doBleThread.detach();
#endif

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

#ifdef CONFIG_ENABLE_BLE
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
#endif

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
