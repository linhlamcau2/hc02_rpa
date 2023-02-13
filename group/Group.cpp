#include "Group.h"
#include <thread>
#include <Log.h>
#include "BleProtocol.h"
#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#endif

#define ID_START (49152)

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

string Group::GetUUId()
{
	return groupUUId;
}

int Group::GetPositionDevice(Device *device)
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
 * @brief Add device function
 *
 * @param device ID Device
 * @param epId Element ID
 * @return true success
 * @return false fail
 */
bool Group::AddDevice(Device *device, int epId, bool sendBle)
{
	// TODO: Check exsit
	// if (std::find(deviceList.begin(), deviceList.end(), device) != deviceList.end())
	// 	return false;
	if (!device)
		return false;

	if (device->GetProtocol() == BLE_DEVICE)
	{
		DeviceInGroup *deviceInGroup = new DeviceInGroup(device, epId);
		if (sendBle)
		{
			if (bleProtocol->AddDev2Group(device->GetAddr(), epId, id + ID_START) == 0)
			{
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
		else
		{
			if (deviceInGroup)
			{
				deviceList.push_back(deviceInGroup);
				numberOfBleDevice++;
			}
			return true;
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

bool Group::DelDevice(Device *device, int epId)
{
	if (device->GetProtocol() == BLE_DEVICE)
	{
		if (bleProtocol->DelDev2Group(device->GetAddr(), epId, id + ID_START) == 0)
		{
			int deviceIndex = GetPositionDevice(device);
			if (deviceIndex > -1)
			{
				deviceList.erase(deviceList.begin() + deviceIndex);
			}
			return true;
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
	return false;
}

bool Group::Do(Json::Value &dataValue)
{
	this->dataValue = dataValue;
	DoBle(&this->dataValue);

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
		bool isIdHue = false;
		bool isIdSaturation = false;
		bool isIdLuminance = false;
		uint16_t valueHue, valueSaturation, valueLuminance;
		for (Json::ArrayIndex i = 0; i < dataValue->size(); i++)
		{
			Json::Value property = dataValue[0][i];
			if (property.isMember("ID") && property["ID"].isInt() &&
					property.isMember("VALUE") && property["VALUE"].isInt())
			{
				int idProperty = property["ID"].asInt();
				unsigned int value = property["VALUE"].asInt();
				if (idProperty == 0)
				{
					bleProtocol->SetOnOffLight(id + ID_START, value, 0, true);
				}
				else if (idProperty == 1)
				{
					bleProtocol->SetDimmingLight(id + ID_START, (value * 65535) / 100, 0, true);
				}
				else if (idProperty == 2)
				{
					bleProtocol->SetCctLight(id + ID_START, (value * 192) + 800, 0, true);
				}
				else if (idProperty == 3)
				{
					isIdHue = true;
					valueHue = value;
				}
				else if (idProperty == 4)
				{
					isIdSaturation = true;
					valueSaturation = value;
				}
				else if (idProperty == 5)
				{
					isIdLuminance = true;
					valueLuminance = value;
				}
				else if (idProperty == 23)
				{
					bleProtocol->CallModeRgb(id + ID_START, value);
				}
				else
				{
					LOGW("DoTrigger id: %d don't support", id);
				}
			}
		}
		if (isIdHue && isIdLuminance && isIdSaturation)
		{
			bleProtocol->SetHSLLight(id + ID_START, valueHue, valueSaturation, valueLuminance, 0, true);
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
