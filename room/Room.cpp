#include "Room.h"
#include "Log.h"
#include "BleProtocol.h"
#include "Db.h"

Room::Room(string id, uint16_t addr, string name) : Group(id, addr, name)
{
	dataConfig = "";
}

Room::~Room()
{
}

int Room::GetPositionGroup(Group *group)
{
	string id = group->GetId();
	mtxGroup.lock();
	for (uint32_t i = 0; i < groupList.size(); i++)
	{
		if (id == groupList[i]->GetId())
		{
			mtxGroup.unlock();
			return i;
		}
	}
	mtxGroup.unlock();
	return CODE_ERROR;
}

int Room::GetPositionSceneBle(SceneBle *sceneBle)
{
	string id = sceneBle->GetId();
	mtxScene.lock();
	for (int i = 0; i < sceneBleList.size(); i++)
	{
		if (id == sceneBleList[i]->GetId())
		{
			mtxScene.unlock();
			return i;
		}
	}
	mtxScene.unlock();
	return CODE_ERROR;
}

string Room::GetDataConfig()
{
	return this->dataConfig;
}

void Room::SetDataConfig(string dataConfig)
{
	this->dataConfig = dataConfig;
}

int Room::AddDevice(Device *device, int epId, bool sendBle, bool addDb)
{
	if (!device)
		return CODE_ERROR;
	if (addDb)
		database->DeviceInRoomAdd(this, device);
	return Group::AddDevice(device, epId, sendBle, false);
}

int Room::DelDevice(Device *device, int epId, bool sendBle, bool delDb)
{
	if (!device)
		return CODE_ERROR;
	if (delDb)
		database->DeviceInRoomDel(this, device);
	return Group::DelDevice(device, epId, sendBle, false);
}

int Room::AddGroup(Group *group, bool isAddGateway, bool isAddDatabase)
{
	if (GetPositionGroup(group) < 0)
	{
		if (isAddGateway)
		{
			mtxGroup.lock();
			if (GetPositionGroup(group) == CODE_ERROR)
				groupList.push_back(group);
			mtxGroup.unlock();
		}
		if (isAddDatabase)
		{
			database->GroupUpdateRoom(group, this->GetId());
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

/**
 * Don't delete in database
 * Function main delete record in database
 */
int Room::DelGroup(Group *group, bool delDb)
{
	int position = GetPositionGroup(group);
	if (position > -1)
	{
		mtxGroup.lock();
		groupList.erase(groupList.begin() + position);
		mtxGroup.unlock();
		if (delDb)
			database->GroupUpdateRoom(group, "");
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Room::AddSceneBle(SceneBle *sceneBle, bool isAddGateway, bool isAddDatabase)
{
	if (GetPositionSceneBle(sceneBle) < 0)
	{
		if (isAddGateway)
		{
			mtxScene.lock();
			if (GetPositionSceneBle(sceneBle) == CODE_ERROR)
				sceneBleList.push_back(sceneBle);
			mtxScene.unlock();
		}
		if (isAddDatabase)
		{
			database->SceneBleUpdateRoom(sceneBle, this->GetId());
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

/**
 * Don't delete in database same DelGroup
 */
int Room::DelSceneBle(SceneBle *sceneBle, bool delDb)
{
	int position = GetPositionSceneBle(sceneBle);
	if (position > -1)
	{
		mtxScene.lock();
		sceneBleList.erase(sceneBleList.begin() + position);
		mtxScene.unlock();
		if (delDb)
			database->SceneBleUpdateRoom(sceneBle, "");
		return CODE_OK;
	}
	return CODE_ERROR;
}