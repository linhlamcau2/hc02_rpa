#include "Group.h"
#include <thread>
#include "Log.h"
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

Group::Group(string id, uint32_t addr, string name) : Object(id, addr, name)
{
}

Group::~Group()
{
	deviceList.clear();
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
	return CODE_ERROR;
}

/**
 * @brief Add device function
 *
 * @param device ID Device
 * @param epId Element ID
 * @return true success
 * @return false fail
 */
int Group::AddDevice(Device *device, int epId, bool sendBle)
{
	// TODO: Check exsit
	// if (std::find(deviceList.begin(), deviceList.end(), device) != deviceList.end())
	// 	return CODE_ERROR;
	if (!device)
		return CODE_ERROR;

	if (device->GetProtocol() == BLE_DEVICE)
	{
		DeviceInGroup *deviceInGroup = new DeviceInGroup(device, epId);
		if (sendBle)
		{
			if (bleProtocol)
			{
				if (bleProtocol->AddDev2Group(device->GetAddr(), epId, addr + ID_START) == CODE_OK)
				{
					if (deviceInGroup)
					{
						deviceList.push_back(deviceInGroup);
						return CODE_OK;
					}
				}
				else
				{
					LOGW("Add Ble device %s to group %d error", device->GetId().c_str(), addr);
				}
			}
			else
				LOGW("BleProtocol null");
		}
		else
		{
			if (deviceInGroup)
			{
				deviceList.push_back(deviceInGroup);
				return CODE_OK;
			}
		}
	}

#ifdef CONFIG_ENABLE_ZIGBEE
	// if (device->GetProtocol() == ZIGBEE_DEVICE)
	// {
	// 	if (zigbeeProtocol->AddGroup(id, device->GetAddr(), epId))
	// 	{
	// 		DeviceInGroup *deviceInGroup = new DeviceInGroup(device, epId);
	// 		if (deviceInGroup)
	// 		{
	// 			deviceList.push_back(deviceInGroup);
	// 			numberOfZigbeeDevice++;
	// 		}
	// 		return CODE_OK;
	// 	}
	// 	else
	// 	{
	// 		LOGW("Add Zigbee device %s to group %d error", device->GetId().c_str(), addr);
	// 	}
	// }
#endif
	return CODE_ERROR;
}

int Group::DelDevice(Device *device, int epId)
{
	if (device->GetProtocol() == BLE_DEVICE)
	{
		if (bleProtocol)
		{
			if (bleProtocol->DelDev2Group(device->GetAddr(), epId, addr + ID_START) == CODE_OK)
			{
				int deviceIndex = GetPositionDevice(device);
				if (deviceIndex > -1)
				{
					deviceList.erase(deviceList.begin() + deviceIndex);
				}
				return CODE_OK;
			}
		}
		else
			LOGW("BleProtocol null");
	}

#ifdef CONFIG_ENABLE_ZIGBEE
	if (device->GetProtocol() == ZIGBEE_DEVICE)
	{
		// TODO: remove from group
		// numberOfZigbeeDevice--;
		// zigbeeProtocol->AddGroup(id, device->GetAddr(), epId);
	}
#endif
	// TODO: remove from list
	// if (device)
	// 	deviceList.erase(remove(deviceList.begin(), deviceList.end(), device), deviceList.end());
	return CODE_ERROR;
}

int Group::Do(Json::Value &dataValue)
{
	this->dataValue = dataValue;
	DoBle();
#ifdef CONFIG_ENABLE_ZIGBEE
	DoZigbee();
#endif
	return CODE_OK;
}

int Group::DoV2(Json::Value &dataValue)
{
	this->dataValue = dataValue;
	DoBleV2();
	// DoZigbeeV2();
	return CODE_OK;
}

void Group::DoBle()
{
	bool isIdHue = false;
	bool isIdSaturation = false;
	bool isIdLuminance = false;
	uint16_t valueHue, valueSaturation, valueLuminance;
	if (bleProtocol)
	{
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			Json::Value property = dataValue[i];
			if (property.isMember("ID") && property["ID"].isInt() &&
				property.isMember("VALUE") && property["VALUE"].isInt())
			{
				int idProperty = property["ID"].asInt();
				unsigned int value = property["VALUE"].asInt();

				if (idProperty == 0)
				{
					bleProtocol->SetOnOffLight(addr + ID_START, value, 5, false);
				}
				else if (idProperty == 1)
				{
					bleProtocol->SetDimmingLight(addr + ID_START, (value * 65535) / 100, 5, false);
				}
				else if (idProperty == 2)
				{
					bleProtocol->SetCctLight(addr + ID_START, (value * 192) + 800, 5, false);
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
					bleProtocol->CallModeRgb(addr + ID_START, value);
				}
				else
				{
					LOGW("DoTrigger addr: %d don't support", addr);
				}
			}
			else
			{
				LOGW("data format err: %s", property.toString().c_str());
			}
		}
		if (isIdHue && isIdLuminance && isIdSaturation)
		{
			bleProtocol->SetHSLLight(addr + ID_START, valueHue, valueSaturation, valueLuminance, 5, false);
		}
	}
	else
		LOGW("BleProtocol null");
}

void Group::DoBleV2()
{
	if (bleProtocol && dataValue.isObject())
	{
		if (dataValue.isMember(KEY_ATTRIBUTE_ONOFF) && dataValue[KEY_ATTRIBUTE_ONOFF].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_ONOFF].asInt();
			bleProtocol->SetOnOffLight(addr + ID_START, value, 0, true);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_DIM) && dataValue[KEY_ATTRIBUTE_DIM].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_DIM].asInt();
			uint16_t dim = (value * 65535) / 100;
			bleProtocol->SetDimmingLight(addr + ID_START, dim, 0, true);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_CCT].asInt();
			uint16_t cct = (value * 192) + 800;
			bleProtocol->SetCctLight(addr + ID_START, cct, 0, true);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_HUE) && dataValue[KEY_ATTRIBUTE_HUE].isInt() &&
			dataValue.isMember(KEY_ATTRIBUTE_SATURATION) && dataValue[KEY_ATTRIBUTE_SATURATION].isInt() &&
			dataValue.isMember(KEY_ATTRIBUTE_LUMINANCE) && dataValue[KEY_ATTRIBUTE_LUMINANCE].isInt())
		{
			int h = dataValue[KEY_ATTRIBUTE_HUE].asInt();
			int s = dataValue[KEY_ATTRIBUTE_SATURATION].asInt();
			int l = dataValue[KEY_ATTRIBUTE_LUMINANCE].asInt();
			bleProtocol->SetHSLLight(addr + ID_START, h, s, l, 0, true);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && dataValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
			bleProtocol->CallModeRgb(addr + ID_START, value);
		}
	}
}

#ifdef CONFIG_ENABLE_ZIGBEE
void Group::DoZigbee()
{
	if (zigbeeProtocol && dataValue.isObject())
	{
		if (dataValue.isMember("method") && dataValue["method"].isString())
		{
			string method = dataValue["method"].asString();
			if (method == "TurnOn")
			{
				zigbeeProtocol->ZCLOnoffGroup(addr, 0);
			}
			else if (method == "TurnOff")
			{
				zigbeeProtocol->ZCLOnoffGroup(addr, 1);
			}
			else if (method == "Toggle")
			{
				zigbeeProtocol->ZCLOnoffGroup(addr, 2);
			}
			else
			{
				LOGW("Zigbee Group not handle method %s", method.c_str());
			}
		}
	}
}
#endif
