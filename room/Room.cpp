#include "Room.h"
#include "Log.h"
#include "BleProtocol.h"
#include "Db.h"

Room::Room(string id, uint32_t addr, string name) : Group(id, addr, name)
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

int Room::AddDevice(Device *device, int epId, bool sendBle)
{
	database->DeviceInRoomAdd(this, device);
	return Group::AddDevice(device, epId, sendBle);
}

int Room::AddGroup(Group *group, bool isAddGateway, bool isAddDatabase)
{
	if (GetPositionGroup(group) < 0)
	{
		if (isAddGateway)
		{
			mtxGroup.lock();
			groupList.push_back(group);
			mtxGroup.unlock();
		}
		if (isAddDatabase)
		{
			database->GroupUpdateRoom(group, id);
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

/**
 * Don't delete in database
 * Function main delete record in database
 */
int Room::DelGroup(Group *group)
{
	int position = GetPositionGroup(group);
	if (position > -1)
	{
		mtxGroup.lock();
		groupList.erase(groupList.begin() + position);
		mtxGroup.unlock();
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
			sceneBleList.push_back(sceneBle);
			mtxScene.unlock();
		}
		if (isAddDatabase)
		{
			database->SceneBleUpdateRoom(sceneBle, id);
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

/**
 * Don't delete in database same DelGroup
 */
int Room::DelSceneBle(SceneBle *sceneBle)
{
	int position = GetPositionSceneBle(sceneBle);
	if (position > -1)
	{
		mtxScene.lock();
		sceneBleList.erase(sceneBleList.begin() + position);
		mtxScene.unlock();
		return CODE_OK;
	}
	return CODE_ERROR;
}
