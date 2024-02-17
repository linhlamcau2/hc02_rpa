#include "Gateway.h"
#include "Db.h"

#define ELEMENT_MAX 4

Device *Gateway::getDeviceFromMac(string mac)
{
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		if (device->GetMac() == mac)
		{
			deviceListMtx.unlock();
			return device;
		}
	}
	deviceListMtx.unlock();
	return NULL;
}

Device *Gateway::getDeviceFromId(string id)
{
	deviceListMtx.lock();
	for (const auto &[idDev, device] : deviceList)
	{
		if (id == idDev)
		{
			deviceListMtx.unlock();
			return device;
		}
	}
	// if (deviceList.find(id) != deviceList.end())
	// {
	// 	deviceListMtx.unlock();
	// 	return deviceList[id];
	// }
	deviceListMtx.unlock();
	return NULL;
}

DeviceBle *Gateway::getDeviceBleFromAddr(uint16_t addr)
{
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		if (device->CheckAddr(addr) && device->GetProtocol() == BLE_DEVICE)
		{
			DeviceBle *deviceBle = dynamic_cast<DeviceBle *>(device);
			if (deviceBle)
			{
				deviceListMtx.unlock();
				return deviceBle;
			}
		}
	}
	deviceListMtx.unlock();
	return NULL;
}

#ifdef CONFIG_ENABLE_ZIGBEE
DeviceZigbee *Gateway::getDeviceZigbeeFromAddr(uint16_t addr)
{
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		if (device->CheckAddr(addr) && device->GetProtocol() == ZIGBEE_DEVICE)
		{
			DeviceZigbee *deviceZigbee = dynamic_cast<DeviceZigbee *>(device);
			if (deviceZigbee)
			{
				deviceListMtx.unlock();
				return deviceZigbee;
			}
		}
	}
	deviceListMtx.unlock();
	return NULL;
}
#endif

// Del dev in all group, scenBle, room
void Gateway::delDevice(Device *device)
{
	deviceListMtx.lock();
	deviceList.erase(device->GetId());
	deviceListMtx.unlock();
	database->DeviceDel(device);

	groupListMtx.lock();
	for (auto &[id, grp] : groupList)
	{
		for (auto &dev : grp->deviceList)
		{
			if (device == dev->device)
			{
				grp->deviceList.erase(remove(grp->deviceList.begin(), grp->deviceList.end(), dev), grp->deviceList.end());
			}
		}
	}
	groupListMtx.unlock();
	database->DeviceInGroupDelDev(device);

	sceneBleListMtx.lock();
	for (auto &[id, sble] : sceneBleList)
	{
		for (auto &dev : sble->deviceList)
		{
			if (device == dev->device)
			{
				sble->deviceList.erase(remove(sble->deviceList.begin(), sble->deviceList.end(), dev), sble->deviceList.end());
			}
		}
	}
	sceneBleListMtx.unlock();
	database->DeviceInSceneBleDelDev(device);

	roomListMtx.lock();
	for (auto &[id, room] : roomList)
	{
		for (auto &dev : room->deviceList)
		{
			if (device == dev->device)
			{
				room->deviceList.erase(remove(room->deviceList.begin(), room->deviceList.end(), dev), room->deviceList.end());
			}
		}
	}
	roomListMtx.unlock();
	database->DeviceInRoomDelDev(device);

	delete device;
}

Group *Gateway::getGroupFromId(string id)
{
	groupListMtx.lock();
	if (groupList.find(id) != groupList.end())
	{
		groupListMtx.unlock();
		return groupList[id];
	}
	groupListMtx.unlock();
	return NULL;
}

Group *Gateway::getGroupFromAddr(uint16_t addr)
{
	groupListMtx.lock();
	for (const auto &[id, group] : groupList)
	{
		if (group->GetAddr() == addr)
		{
			groupListMtx.unlock();
			return group;
		}
	}
	groupListMtx.unlock();
	return NULL;
}

void Gateway::delGroup(Group *group)
{
	groupListMtx.lock();
	groupList.erase(group->GetId());
	groupListMtx.unlock();
	database->GroupDel(group);

	roomListMtx.lock();
	for (auto &[id, room] : roomList)
	{
		for (auto &grp : room->groupList)
		{
			if (grp == group)
			{
				room->groupList.erase(remove(room->groupList.begin(), room->groupList.end(), grp), room->groupList.end());
			}
		}
	}
	roomListMtx.unlock();

	delete group;
}

uint16_t Gateway::getNextGroupAddr()
{
	uint16_t groupAddr = 10000; // start add of normal group
	groupListMtx.lock();
	for (const auto &[id, group] : groupList)
	{
		if (group->GetAddr() >= groupAddr)
		{
			groupAddr = group->GetAddr() + 1;
		}
	}
	groupListMtx.unlock();
	return groupAddr;
}

SceneBle *Gateway::getSceneBleFromId(string id)
{
	sceneBleListMtx.lock();
	if (sceneBleList.find(id) != sceneBleList.end())
	{
		sceneBleListMtx.unlock();
		return sceneBleList[id];
	}
	sceneBleListMtx.unlock();
	return NULL;
}

SceneBle *Gateway::getSceneBleFromAddr(uint16_t addr)
{
	sceneBleListMtx.lock();
	for (const auto &[id, sceneBle] : sceneBleList)
	{
		if (sceneBle->GetAddr() == addr)
		{
			sceneBleListMtx.unlock();
			return sceneBle;
		}
	}
	sceneBleListMtx.unlock();
	return NULL;
}

void Gateway::delSceneBle(SceneBle *sceneBle)
{
	sceneBleListMtx.lock();
	sceneBleList.erase(sceneBle->GetId());
	sceneBleListMtx.unlock();
	database->SceneBleDel(sceneBle);

	roomListMtx.lock();
	for (auto &[id, room] : roomList)
	{
		for (auto &sBle : room->sceneBleList)
		{
			if (sBle == sceneBle)
			{
				room->sceneBleList.erase(remove(room->sceneBleList.begin(), room->sceneBleList.end(), sBle), room->sceneBleList.end());
			}
		}
	}
	roomListMtx.unlock();

	delete sceneBle;
}

uint16_t Gateway::getNextSceneBleAddr()
{
	uint16_t sceneAddr = 10000;
	sceneBleListMtx.lock();
	for (const auto &[id, sceneBle] : sceneBleList)
	{
		if (sceneBle->GetAddr() >= sceneAddr)
		{
			sceneAddr = sceneBle->GetAddr() + 1;
		}
	}
	sceneBleListMtx.unlock();
	return sceneAddr;
}

Rule *Gateway::getRuleFromId(string id)
{
	ruleListMtx.lock();
	if (ruleList.find(id) != ruleList.end())
	{
		ruleListMtx.unlock();
		return ruleList[id];
	}
	ruleListMtx.unlock();
	return NULL;
}

void Gateway::delRule(Rule *rule)
{
	ruleListMtx.lock();
	ruleList.erase(rule->GetId());
	ruleListMtx.unlock();
	database->RuleDel(rule);
	delete rule;
}

Room *Gateway::getRoomFromId(string id)
{
	roomListMtx.lock();
	if (roomList.find(id) != roomList.end())
	{
		roomListMtx.unlock();
		return roomList[id];
	}
	roomListMtx.unlock();
	return NULL;
}

void Gateway::delRoom(Room *room)
{
	roomListMtx.lock();
	roomList.erase(room->GetId());
	roomListMtx.unlock();
	database->RoomDel(room);

	groupListMtx.lock();
	groupList.erase(room->GetId());
	groupListMtx.unlock();

	delete room;
}

uint16_t Gateway::getNextRoomAddr()
{
	uint16_t roomAddr = 1; // start add of room
	groupListMtx.lock();
	for (const auto &[id, group] : groupList)
	{
		if (group->GetAddr() >= roomAddr)
		{
			roomAddr = (group->GetAddr() / 200 + 1) * 200; // every room has 200 group
		}
	}
	groupListMtx.unlock();
	return roomAddr;
}

uint32_t Gateway::GetNextAndroidProvisionAddr()
{
	uint32_t nextAddr = 20000;
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		if (device->GetAddr() > nextAddr)
		{
			nextAddr = device->GetAddr() + ELEMENT_MAX;
		}
	}
	deviceListMtx.unlock();
	return nextAddr;
}

uint32_t Gateway::GetMaxAddrBle()
{
	uint32_t nextAddr = 2;
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		if ((device->GetAddr() > nextAddr) && (device->GetAddr() < 49152))
		{
			nextAddr = device->GetAddr();
		}
	}
	deviceListMtx.unlock();
	return nextAddr;
}

map<string, Device *> Gateway::GetListDevices()
{
	map<string, Device *> listDevs;
	deviceListMtx.lock();
	listDevs = deviceList;
	deviceListMtx.unlock();
	return listDevs;
}
