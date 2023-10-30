#include "Gateway.h"
#include "Log.h"
#include "Db.h"

void Gateway::InitMqttMessageGroup()
{
	OnDeviceRpcCallbackRegister("controlGroup", bind(&Gateway::OnControlGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("createGroup", bind(&Gateway::OnCreateGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("addDevToGroup", bind(&Gateway::OnAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("delDevFromGroup", bind(&Gateway::OnDeleteDeviceFromGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("delGroup", bind(&Gateway::OnDeleteGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getGroupList", bind(&Gateway::OnGetGroupList, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getDevListInGroup", bind(&Gateway::OnGetDevListInGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("updateGroupName", bind(&Gateway::OnUpdateGroupName, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("controlGroup", bind(&Gateway::OnControlGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("createGroup", bind(&Gateway::OnCreateGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("addDevToGroup", bind(&Gateway::OnAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delDevFromGroup", bind(&Gateway::OnDeleteDeviceFromGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delGroup", bind(&Gateway::OnDeleteGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getGroupList", bind(&Gateway::OnGetGroupList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getDevListInGroup", bind(&Gateway::OnGetDevListInGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("updateGroupName", bind(&Gateway::OnUpdateGroupName, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnControlGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlGroup");
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("data") && reqValue["data"].isObject())
	{
		string groupId = reqValue["id"].asString();
		Json::Value devData = reqValue["data"];
		Group *group = getGroupFromId(groupId);
		if (group)
		{
			int rs = group->Do(devData);
			respValue["data"]["code"] = rs;
		}
		else
		{
			LOGW("Group id %s not found", groupId.c_str());
			respValue["data"]["code"] = CODE_NOT_FOUND_GROUP;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlGroup %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "controlGroupRsp";
	return CODE_OK;
}

int Gateway::OnGetGroupList(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetGroupList");
	Json::Value groupData;
	groupListMtx.lock();
	for (const auto &[id, group] : groupList)
	{
		Json::Value groupValue;
		groupValue["id"] = group->GetId();
		groupValue["name"] = group->GetName();
		groupData.append(groupValue);
	}
	groupListMtx.unlock();
	respValue["data"]["groups"] = groupData;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getGroupListRsp";
	return CODE_OK;
}

int Gateway::OnGetDevListInGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetDevListInGroup");
	if (reqValue.isMember("groups") && reqValue["groups"].isArray() && reqValue["groups"].size() > 0)
	{
		Json::Value groupList;
		Json::Value groups = reqValue["groups"];
		for (auto &groupValue : groups)
		{
			Json::Value groupsData;
			if (groupValue.isString())
			{
				string id = groupValue.asString();
				groupsData["id"] = id;
				Group *temp = getGroupFromId(id);
				if (temp)
				{
					Json::Value temp_devicesList;
					for (unsigned int i = 0; i < temp->deviceList.size(); i++)
					{
						DeviceInGroup *deviceInGroup = temp->deviceList[i];
						string deviceId = deviceInGroup->device->GetId();
						temp_devicesList.append(deviceId);
					}
					groupsData["devices"] = temp_devicesList;
				}
				else
					respValue["data"]["code"] = CODE_NOT_FOUND_GROUP;
			}
			else
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
			groupList.append(groupsData);
		}
		respValue["data"]["groups"] = groupList;
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getDevListInGroupRsp";
	return CODE_OK;
}

int Gateway::OnAddDeviceGroupBle(Json::Value &deviceList, Json::Value &respSuccessList, Json::Value &respFailList, Group *group)
{
	LOGD("OnAddDeviceGroupBle");
	for (unsigned int i = 0; i < deviceList.size(); i++)
	{
		if (deviceList[i].isString())
		{
			string deviceId = deviceList[i].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				int deviceAddr = device->GetAddr();
				if (group->AddDevice(device, deviceAddr, true, true) == CODE_OK)
				{
					respSuccessList.append(deviceId);
				}
				else
				{
					LOGD("add group deviceId %s error", deviceId.c_str());
					respFailList.append(deviceId);
				}
			}
			else
			{
				LOGD("deviceId %s dose not exist", deviceId.c_str());
				respFailList.append(deviceId);
			}
		}
	}
	return CODE_OK;
}

int Gateway::OnCreateGroup(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("name") && reqValue["name"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string groupId = reqValue["id"].asString();
		string groupName = reqValue["name"].asString();
		Json::Value devicesValue = reqValue["devices"];
		// TODO: add start address of normal group
		string roomId;
		Group *group = new Group(groupId, getNextGroupAddr(), groupName);
		if (group)
		{
			if (AddNewGroup(group, true))
			{
				if (reqValue.isMember("roomId") && reqValue["roomId"].isString())
				{
					roomId = reqValue["roomId"].asString();
					Room *room = getRoomFromId(roomId);
					if (room)
					{
						room->AddGroup(group, true, true);
					}
				}
				for (auto &deviceValue : devicesValue)
				{
					if (deviceValue.isString())
					{
						string deviceId = deviceValue.asString();
						Device *device = getDeviceFromId(deviceId);
						if (device)
						{
							int deviceAddr = device->GetAddr();
							if (group->AddDevice(device, deviceAddr, true, true) == CODE_OK)
							{
								successList.append(deviceId);
							}
							else
							{
								LOGD("add group deviceId %s error", deviceId.c_str());
								failedList.append(deviceId);
							}
						}
						else
						{
							LOGD("deviceId %s dose not exist", deviceId.c_str());
							failedList.append(deviceId);
						}
					}
				}
				respValue["data"]["code"] = CODE_OK;
				respValue["data"]["addr"] = group->GetAddr();
				respValue["data"]["id"] = groupId;
				respValue["data"]["success"] = successList;
				respValue["data"]["failed"] = failedList;
				printGroup();
				pushMsgHcCoreToHcApp("createGroup", groupId, groupName, successList, roomId);
			}
			else
			{
				respValue["data"]["code"] = CODE_DATABASE_ERROR;
			}
		}
		else
		{
			respValue["data"]["code"] = CODE_MEMORY_ERROR;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "createGroupRsp";
	return CODE_OK;
}

int Gateway::OnAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string groupId = reqValue["id"].asString();
		Json::Value devicesValue = reqValue["devices"];
		Group *group = getGroupFromId(groupId);
		if (group)
		{
			for (auto &deviceValue : devicesValue)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						int deviceAddr = device->GetAddr();
						if (group->AddDevice(device, deviceAddr, true, true) == CODE_OK)
						{
							successList.append(deviceId);
						}
						else
						{
							LOGD("add to group deviceId %s error", deviceId.c_str());
							failedList.append(deviceId);
						}
					}
					else
					{
						LOGD("deviceId %s dose not exist", deviceId.c_str());
						failedList.append(deviceId);
					}
				}
			}
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
			printGroup();
			pushMsgHcCoreToHcApp("addDevToGroup", groupId, group->GetName(), successList, "");
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_GROUP;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "addDevToGroupRsp";
	return CODE_OK;
}

int Gateway::OnDelDeviceGroupBle(Json::Value &deviceList, Json::Value &respSuccessList, Json::Value &respFailList, Group *group)
{
	for (unsigned int i = 0; i < deviceList.size(); i++)
	{
		if (deviceList[i].isString())
		{
			string deviceId = deviceList[i].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				int deviceAddr = device->GetAddr();
				if (group->DelDevice(device, deviceAddr, true, true) == CODE_OK)
				{
					// database->DeviceInGroupDel(group, device, deviceAddr);
					respSuccessList.append(deviceId);
				}
				else
				{
					LOGD("delete from group deviceId %s error", deviceId.c_str());
					respFailList.append(deviceId);
				}
			}
			else
			{
				LOGD("deviceId %s dose not exist", deviceId.c_str());
				respFailList.append(deviceId);
			}
		}
	}
	return CODE_OK;
}

int Gateway::OnDeleteDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string groupId = reqValue["id"].asString();
		Json::Value devicesValue = reqValue["devices"];
		Group *group = getGroupFromId(groupId);
		if (group)
		{
			for (auto &deviceValue : devicesValue)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						int deviceAddr = device->GetAddr();
						if (group->DelDevice(device, deviceAddr, true, true) == CODE_OK)
						{
							// database->DeviceInGroupDel(group, device, deviceAddr);
							successList.append(deviceId);
						}
						else
						{
							LOGD("delete from group deviceId %s error", deviceId.c_str());
							failedList.append(deviceId);
						}
					}
					else
					{
						LOGD("deviceId %s dose not exist", deviceId.c_str());
						failedList.append(deviceId);
					}
				}
			}
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
			printGroup();
			pushMsgHcCoreToHcApp("delDevFromGroup", groupId, group->GetName(), successList, "");
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_GROUP;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "delDevFromGroupRsp";
	return CODE_OK;
}

int Gateway::OnDeleteGroup(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		Json::Value successList;
		Json::Value failedList;
		string groupId = reqValue["id"].asString();
		Group *group = getGroupFromId(groupId);
		if (group)
		{
			vector<DeviceInGroup *> devicesInGroup = group->deviceList;
			for (auto &deviceInGroup : devicesInGroup)
			{
				if (group->DelDevice(deviceInGroup->device, deviceInGroup->device->GetAddr(), true, true) == CODE_OK)
				{
					// database->DeviceInGroupDel(group, deviceInGroup->device, deviceInGroup->device->GetAddr());
					successList.append(deviceInGroup->device->GetId());
				}
				else
				{
					failedList.append(deviceInGroup->device->GetId());
				}
			}

			pushMsgHcCoreToHcApp("delGroup", groupId, group->GetName(), successList, "");
			delGroup(group);
			printGroup();

			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_GROUP;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "delGroupRsp";
	return CODE_OK;
}

int Gateway::OnUpdateGroupName(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("name") && reqValue["name"].isString() && reqValue.isMember("id") && reqValue["id"].isString())
	{
		string id = reqValue["id"].asString();
		string name = reqValue["name"].asString();
		Group *group = getGroupFromId(id);
		if (group)
		{
			group->SetName(name);
			database->GroupUpdate(group);
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "updateGroupNameRsp";
	return CODE_OK;
}
