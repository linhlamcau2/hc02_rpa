#include "Group.h"
#include <thread>
#include "Log.h"
#include "BleProtocol.h"
#include "Db.h"
#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#endif

DeviceInGroup::DeviceInGroup(Device *device, int epId)
{
	this->device = device;
	this->epId = epId;
}

Group::Group(string id, uint16_t addr, string name) : Object(id, addr, name)
{
}

Group::~Group()
{
	mtx.lock();
	deviceList.clear();
	mtx.unlock();
}

int Group::GetPositionDevice(Device *device, int epid)
{
	string deviceId = device->GetId();
	mtx.lock();
	for (uint32_t i = 0; i < deviceList.size(); i++)
	{
		if ((deviceId == deviceList[i]->device->GetId()) && (deviceList[i]->epId == epid))
		{
			mtx.unlock();
			return i;
		}
	}
	mtx.unlock();
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
int Group::AddDevice(Device *device, int epId, bool sendBle, bool addDb)
{
	if (!device)
		return CODE_ERROR;

	// // TODO: Check exsit
	// if (std::find(deviceList.begin(), deviceList.end(), device) != deviceList.end())
	// 	return CODE_ERROR;
	DeviceInGroup *deviceInGroup = new DeviceInGroup(device, epId);
	if (!deviceInGroup)
		return CODE_ERROR;

	if (GetPositionDevice(device, epId) == CODE_ERROR)
	{
		if (device->GetProtocol() == BLE_DEVICE)
		{
			if (sendBle)
			{
				if (bleProtocol)
				{
					if (bleProtocol->AddDev2Group(device->GetAddr(), epId, addr + ID_START) == CODE_OK)
					{
						mtx.lock();
						deviceList.push_back(deviceInGroup);
						mtx.unlock();

						if (addDb)
							database->DeviceInGroupAdd(this, device, epId);
						return CODE_OK;
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
				mtx.lock();
				deviceList.push_back(deviceInGroup);
				mtx.unlock();

				if (addDb)
					database->DeviceInGroupAdd(this, device, epId);
				return CODE_OK;
			}
		}
		else
		{
			mtx.lock();
			deviceList.push_back(deviceInGroup);
			mtx.unlock();
			return CODE_OK;
		}
	}
	else
	{
		LOGW("Device %s is exist in group", device->GetId().c_str());
		return CODE_OK;
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

int Group::DelDevice(Device *device, int epId, bool sendBle, bool delDb)
{
	if (!device)
		return CODE_ERROR;

	if (delDb)
		database->DeviceInGroupDel(this, device, epId);
	if (GetPositionDevice(device, epId) != CODE_ERROR)
	{
		if (device->GetProtocol() == BLE_DEVICE)
		{
			if (sendBle)
			{
				if (bleProtocol)
				{
					if (bleProtocol->DelDev2Group(device->GetAddr(), epId, addr + ID_START) == CODE_OK)
					{
						int deviceIndex = GetPositionDevice(device, epId);
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
					LOGW("BleProtocol null");
			}
			else
			{
				int deviceIndex = GetPositionDevice(device, epId);
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
	else
	{
		LOGW("Device %s is not exist in group", device->GetId().c_str());
		return CODE_OK;
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

int Group::Do(Json::Value &dataValue, bool ack)
{
	if (bleProtocol && dataValue.isObject())
	{
		if (dataValue.isMember(KEY_ATTRIBUTE_ONOFF) && dataValue[KEY_ATTRIBUTE_ONOFF].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_ONOFF].asInt();
			bleProtocol->SetOnOffLight(addr + ID_START, value, TRANSITION_DEFAULT, ack);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_DIM) && dataValue[KEY_ATTRIBUTE_DIM].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_DIM].asInt();
			uint16_t dim = (value * 65535) / 100;
			bleProtocol->SetDimmingLight(addr + ID_START, dim, TRANSITION_DEFAULT, ack);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_CCT].asInt();
			uint16_t cct = (value * 192) + 800;
			bleProtocol->SetCctLight(addr + ID_START, cct, TRANSITION_DEFAULT, ack);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_HUE) && dataValue[KEY_ATTRIBUTE_HUE].isInt() &&
			dataValue.isMember(KEY_ATTRIBUTE_SATURATION) && dataValue[KEY_ATTRIBUTE_SATURATION].isInt() &&
			dataValue.isMember(KEY_ATTRIBUTE_LUMINANCE) && dataValue[KEY_ATTRIBUTE_LUMINANCE].isInt())
		{
			int h = dataValue[KEY_ATTRIBUTE_HUE].asInt();
			int s = dataValue[KEY_ATTRIBUTE_SATURATION].asInt();
			int l = dataValue[KEY_ATTRIBUTE_LUMINANCE].asInt();
			bleProtocol->SetHSLLight(addr + ID_START, h, s, l, TRANSITION_DEFAULT, ack);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && dataValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
			bleProtocol->CallModeRgb(addr + ID_START, value);
		}
	}
	return CODE_OK;
}

#ifdef CONFIG_ENABLE_ZIGBEE
// void Group::DoZigbee()
// {
// 	if (zigbeeProtocol && dataValue.isObject())
// 	{
// 		if (dataValue.isMember("method") && dataValue["method"].isString())
// 		{
// 			string method = dataValue["method"].asString();
// 			if (method == "TurnOn")
// 			{
// 				zigbeeProtocol->ZCLOnoffGroup(addr, 0);
// 			}
// 			else if (method == "TurnOff")
// 			{
// 				zigbeeProtocol->ZCLOnoffGroup(addr, 1);
// 			}
// 			else if (method == "Toggle")
// 			{
// 				zigbeeProtocol->ZCLOnoffGroup(addr, 2);
// 			}
// 			else
// 			{
// 				LOGW("Zigbee Group not handle method %s", method.c_str());
// 			}
// 		}
// 	}
// }
#endif
