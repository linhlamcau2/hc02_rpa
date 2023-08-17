#include "Gateway.h"
#include "Log.h"
#include "Db.h"

void Gateway::InitMqttMessageRoom()
{
	// OnDeviceRpcCallbackRegister("addDevToRoom", bind(&Gateway::OnAddDeviceToRoom, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delDevFromRoom", bind(&Gateway::OnDeleteDeviceFromRoom, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delRoom", bind(&Gateway::OnDeleteRoom, this, placeholders::_1, placeholders::_2));

	// OnLocalCallbackRegister("addDevToRoom", bind(&Gateway::OnAddDeviceToRoom, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delDevFromRoom", bind(&Gateway::OnDeleteDeviceFromRoom, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delRoom", bind(&Gateway::OnDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getRoomList", bind(&Gateway::OnGetRoomList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getDevListInRoom", bind(&Gateway::OnGetDevListInRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("createRoom", bind(&Gateway::OnCreateRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("addDevToRoom", bind(&Gateway::OnAddDeviceToRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delDevToRoom", bind(&Gateway::OnDeleteDeviceFromRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delRoom", bind(&Gateway::OnDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getGroupIntoRoom", bind(&Gateway::OnGetGroupIntoRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getSceneIntoRoom", bind(&Gateway::OnGetSceneIntoRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("updateRoomName", bind(&Gateway::OnUpdateRoomName, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnGetRoomList(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetRoomList");
	Json::Value roomData;
	roomListMtx.lock();
	for (const auto &[id, room] : roomList)
	{
		Json::Value roomValue;
		roomValue["id"] = room->GetId();
		roomValue["name"] = room->GetName();
		roomData.append(roomValue);
	}
	roomListMtx.unlock();
	respValue["data"]["rooms"] = roomData;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getRoomListRsp";
	return CODE_OK;
}

int Gateway::OnGetDevListInRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetDeviceStatus");
	if (reqValue.isMember("rooms") && reqValue["rooms"].isArray() && reqValue["rooms"].size() > 0)
	{
		Json::Value roomsData;
		Json::Value rooms = reqValue["rooms"];
		for (auto &roomValue : rooms)
		{
			Json::Value temp_roomsData;
			if (roomValue.isString())
			{
				string roomId = roomValue.asString();
				temp_roomsData["id"] = roomId;
				Room *temp_room = getRoomFromId(roomId);
				if (temp_room)
				{
					Json::Value temp_devicesList;
					for (unsigned int i = 0; i < temp_room->deviceList.size(); i++)
					{
						Json::Value device;
						DeviceInGroup *deviceInRoom = temp_room->deviceList[i];
						string deviceId = deviceInRoom->device->GetId();
						Device *tempDev = getDeviceFromId(deviceId);
						device["id"] = deviceId;
						device["name"] = tempDev->GetName();
						device["type"] = tempDev->GetType();
						temp_devicesList.append(device);
					}
					temp_roomsData["devices"] = temp_devicesList;
					roomsData.append(temp_roomsData);
				}
				else
					respValue["data"]["code"] = CODE_NOT_FOUND_ROOM;
			}
			else
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
		}
		respValue["data"]["rooms"] = roomsData;
	}
	else
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getDevListInRoomRsp";
	return CODE_OK;
}

int Gateway::OnCreateRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
			reqValue.isMember("name") && reqValue["name"].isString() &&
			reqValue.isMember("devices") && reqValue["devices"].isArray() &&
			reqValue.isMember("groups") && reqValue["groups"].isArray() &&
			reqValue.isMember("scenes") && reqValue["scenes"].isArray())
	{
		Json::Value successList;
		Json::Value failedList;
		string roomId = reqValue["id"].asString();
		string roomName = reqValue["name"].asString();
		Json::Value devicesValue = reqValue["devices"];
		Json::Value groupsValue = reqValue["groups"];
		Json::Value scenesValue = reqValue["scenes"];
		respValue["data"]["code"] = CODE_ERROR;
		Room *room = getRoomFromId(roomId);
		if (!room)
		{
			room = new Room(roomId, getNextGroupAddr(), roomName);
			if (room)
			{
				gateway->AddNewRoom(room, true);
				for (auto &deviceValue : devicesValue)
				{
					if (deviceValue.isString())
					{
						string deviceId = deviceValue.asString();
						Device *device = getDeviceFromId(deviceId);
						if (device)
						{
							room->AddDevice(device, device->GetAddr(), true, true);
							successList.append(device->GetId());
						}
						else
						{
							failedList.append(deviceId);
						}
					}
				}

				for (auto &groupValue : groupsValue)
				{
					if (groupValue.isObject())
					{
						if (groupValue.isMember("id") && groupValue["id"].isString() &&
								groupValue.isMember("name") && groupValue["name"].isString() &&
								groupValue.isMember("type") && groupValue["type"].isInt())
						{
							string id = groupValue["id"].asString();
							string name = groupValue["name"].asString();
							int type = groupValue["type"].asInt();
							Group *group = getGroupFromId(id);
							if (!group)
							{
								group = new Group(id, getNextGroupAddr(), name);
								if (group)
								{
									if (AddNewGroup(group, true))
										room->AddGroup(group, true, true);
									for (auto &deviceInRoom : room->deviceList)
									{
										if (deviceInRoom->device->GetType() == type)
										{
											group->AddDevice(deviceInRoom->device, deviceInRoom->device->GetAddr(), true, true);
										}
									}
								}
							}
							else
							{
								respValue["data"]["code"] = CODE_ERROR;
								LOGW("Group id %s exist", id.c_str());
							}
						}
					}
				}

				for (auto &sceneValue : scenesValue)
				{
					if (sceneValue.isObject())
					{
						if (sceneValue.isMember("id") && sceneValue["id"].isString() &&
								sceneValue.isMember("name") && sceneValue["name"].isString() &&
								sceneValue.isMember("groups") && sceneValue["groups"].isArray())
						{
							string id = sceneValue["id"].asString();
							string name = sceneValue["name"].asString();
							Json::Value groupsValue = sceneValue["groups"];
							SceneBle *sceneBle = new SceneBle(id, getNextSceneBleAddr(), name);
							if (sceneBle)
							{
								AddNewSceneBle(sceneBle, true);
								room->AddSceneBle(sceneBle, true, true);
								for (auto &groupValue : groupsValue)
								{
									if (groupValue.isObject())
									{
										if (groupValue.isMember("id") && groupValue["id"].isString() &&
												groupValue.isMember("data") && groupValue["data"].isObject())
										{
											string id = groupValue["id"].asString();
											Json::Value groupData = groupValue["data"];
											Group *group = getGroupFromId(id);
											if (group)
											{
												group->Do(groupData);
												for (auto &deviceInGroup : group->deviceList)
												{
													if (sceneBle->AddDevice(deviceInGroup->device, groupData, true, true) == CODE_OK)
													{
														// database->DeviceInSceneBleAdd(sceneBle, deviceInGroup->device, groupData.toString());
														successList.append(deviceInGroup->device->GetId());
													}
													else
													{
														failedList.append(deviceInGroup->device->GetId());
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}

				respValue["data"]["code"] = CODE_OK;
				respValue["data"]["id"] = roomId;
				respValue["data"]["success"] = successList;
				respValue["data"]["failed"] = failedList;
			}
			else
			{
				respValue["data"]["code"] = CODE_MEMORY_ERROR;
			}
		}
		else
		{
			respValue["data"]["code"] = CODE_ERROR;
			LOGW("Room id %s exist", roomId.c_str());
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "createRoomRsp";
	return CODE_OK;
}

int Gateway::OnAddDeviceToRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
			reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList;
		Json::Value failedList;
		string roomId = reqValue["id"].asString();
		Json::Value devicesValue = reqValue["devices"];
		Room *room = getRoomFromId(roomId);
		if (room)
		{
			for (auto &deviceValue : devicesValue)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						room->AddDevice(device, device->GetAddr(), true, true);
						successList.append(device->GetId());
					}
					else
					{
						failedList.append(deviceId);
					}
				}
			}

			if (reqValue.isMember("groups") && reqValue["groups"].isArray())
			{
				Json::Value groupsValue = reqValue["groups"];
				for (auto &groupValue : groupsValue)
				{
					if (groupValue.isObject())
					{
						if (groupValue.isMember("id") && groupValue["id"].isString() &&
								groupValue.isMember("type") && groupValue["type"].isInt())
						{
							string id = groupValue["id"].asString();
							int type = groupValue["type"].asInt();
							Group *group = getGroupFromId(id);
							if (!group)
							{
								if (groupValue.isMember("name") && groupValue["name"].isString())
								{
									string name = groupValue["name"].asString();
									group = new Group(id, getNextGroupAddr(), name);
									if (group)
									{
										if (AddNewGroup(group, true))
											room->AddGroup(group, true, true);
									}
									else
									{
										respValue["data"]["code"] = CODE_MEMORY_ERROR;
										LOGW("Malloc err");
									}
								}
								else
								{
									respValue["data"]["code"] = CODE_FORMAT_ERROR;
									LOGW("Message format err: Group name not exist");
								}
							}
							if (group)
							{
								for (auto &deviceInRoom : room->deviceList)
								{
									if (deviceInRoom->device->GetType() == type)
									{
										group->AddDevice(deviceInRoom->device, deviceInRoom->device->GetAddr(), true, true);
									}
								}
							}
							else
							{
								LOGW("Group id %s not exist", id.c_str());
							}
						}
					}
				}
			}

			if (reqValue.isMember("scenes") && reqValue["scenes"].isArray())
			{
				Json::Value scenesValue = reqValue["scenes"];
				for (auto &sceneValue : scenesValue)
				{
					if (sceneValue.isObject())
					{
						if (sceneValue.isMember("id") && sceneValue["id"].isString() &&
								sceneValue.isMember("groups") && sceneValue["groups"].isArray())
						{
							string id = sceneValue["id"].asString();
							Json::Value groupsValue = sceneValue["groups"];
							SceneBle *sceneBle = getSceneBleFromId(id);
							if (!sceneBle)
							{
								if (sceneValue.isMember("name") && sceneValue["name"].isString())
								{
									string name = sceneValue["name"].asString();
									sceneBle = new SceneBle(id, getNextSceneBleAddr(), name);
									if (sceneBle)
									{
										room->AddSceneBle(sceneBle, true, true);
										AddNewSceneBle(sceneBle, true);
									}
									else
									{
										respValue["data"]["code"] = CODE_MEMORY_ERROR;
										LOGW("Malloc err");
									}
								}
								else
								{
									respValue["data"]["code"] = CODE_FORMAT_ERROR;
									LOGW("Message format err: Scene name not exist");
								}
							}
							if (sceneBle)
							{
								for (auto &groupValue : groupsValue)
								{
									if (groupValue.isObject())
									{
										if (groupValue.isMember("id") && groupValue["id"].isString() &&
												groupValue.isMember("data") && groupValue["data"].isObject())
										{
											string id = groupValue["id"].asString();
											Json::Value groupData = groupValue["data"];
											Group *group = getGroupFromId(id);
											if (group)
											{
												group->Do(groupData);
												for (auto &deviceInGroup : group->deviceList)
												{
													if (sceneBle->AddDevice(deviceInGroup->device, groupData, true, true) == CODE_OK)
													{
														// database->DeviceInSceneBleAdd(sceneBle, deviceInGroup->device, groupData.toString());
														successList.append(deviceInGroup->device->GetId());
													}
													else
													{
														failedList.append(deviceInGroup->device->GetId());
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}

			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
		}
		else
		{
			respValue["data"]["code"] = CODE_ERROR;
			LOGW("Room id %s not exist", roomId.c_str());
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "addDevToRoomRsp";
	return CODE_OK;
}

int Gateway::OnDeleteDeviceFromRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
			reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList;
		Json::Value failedList;
		string roomId = reqValue["id"].asString();
		Json::Value devicesValue = reqValue["devices"];
		Room *room = getRoomFromId(roomId);
		if (room)
		{
			for (auto &deviceValue : devicesValue)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						if (room->DelDevice(device, device->GetAddr(), true, true) == CODE_OK)
						{
							// database->DeviceInRoomDel(room, device);
							successList.append(deviceId);
						}
						else
						{
							LOGD("delete from room deviceId %s error", deviceId.c_str());
							failedList.append(deviceId);
						}

						for (auto &group : room->groupList)
						{
							for (auto &devInGr : group->deviceList)
							{
								if (devInGr->device->GetId() == device->GetId())
								{
									if (group->DelDevice(device, device->GetAddr(), true, true) == CODE_OK)
									{
										// database->DeviceInGroupDel(group, device, device->GetAddr());
									}
								}
							}
						}

						for (auto &sceneBle : room->sceneBleList)
						{
							for (auto &devInScene : sceneBle->deviceList)
							{
								if (devInScene->device->GetId() == device->GetId())
								{
									if (sceneBle->DelDevice(device, true, true) == CODE_OK)
									{
										// database->DeviceInSceneBleDel(sceneBle, device);
									}
								}
							}
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
	respValue["cmd"] = "delDevFromRoomRsp";
	return CODE_OK;
}

int Gateway::OnDeleteRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		Json::Value successList;
		Json::Value failedList;
		string roomId = reqValue["id"].asString();
		Room *room = getRoomFromId(roomId);
		if (room)
		{
			for (auto &deviceInRoom : room->deviceList)
			{
				if (room->DelDevice(deviceInRoom->device, deviceInRoom->device->GetAddr(), true, true) == CODE_OK)
				{
					// database->DeviceInRoomDel(room, deviceInRoom->device);
					successList.append(deviceInRoom->device->GetId());
				}
				else
				{
					LOGD("delete from room deviceId %s error", deviceInRoom->device->GetId().c_str());
					failedList.append(deviceInRoom->device->GetId());
				}
			}

			// Delete all group
			for (auto &groupInRoom : room->groupList)
			{
				for (auto &deviceInGroup : groupInRoom->deviceList)
				{
					if (groupInRoom->DelDevice(deviceInGroup->device, deviceInGroup->device->GetAddr(), true, true) == CODE_OK)
					{
						// database->DeviceInGroupDel(groupInRoom, deviceInGroup->device, deviceInGroup->device->GetAddr());
					}
				}
				delGroup(groupInRoom);
			}

			// Delete all scene
			for (auto &sceneInRoom : room->sceneBleList)
			{
				for (auto &deviceInScene : sceneInRoom->deviceList)
				{
					if (sceneInRoom->DelDevice(deviceInScene->device, true, true) == CODE_OK)
					{
						// database->DeviceInSceneBleDel(sceneInRoom, deviceInScene->device);
					}
				}
				delSceneBle(sceneInRoom);
			}

			delRoom(room);
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
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
	respValue["cmd"] = "delRoomRsp";
	return CODE_OK;
}

int Gateway::OnGetGroupIntoRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("rooms") && reqValue["rooms"].isArray())
	{
		Json::Value rooms = reqValue["rooms"];
		Json::Value roomsRsp;
		for (auto &room : rooms)
		{
			if (room.isString())
			{
				string roomId = room.asString();
				Room *temp_room = getRoomFromId(roomId);
				if (temp_room)
				{
					Json::Value roomRsp;
					roomRsp["id"] = roomId;
					for (unsigned int i = 0; i < temp_room->groupList.size(); i++)
					{
						Json::Value groupRsp;
						groupRsp["id"] = temp_room->groupList[i]->GetId();
						groupRsp["name"] = temp_room->groupList[i]->GetName();
						roomRsp["groups"].append(groupRsp);
					}
					respValue["data"]["rooms"].append(roomRsp);
					respValue["data"]["code"] = CODE_OK;
				}
			}
		}
	}
	else
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	respValue["cmd"] = "getGroupIntoRoomRsp";
	return CODE_OK;
}

int Gateway::OnGetSceneIntoRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("rooms") && reqValue["rooms"].isArray())
	{
		Json::Value rooms = reqValue["rooms"];
		Json::Value roomsRsp;
		for (auto &room : rooms)
		{
			if (room.isString())
			{
				string roomId = room.asString();
				Room *temp_room = getRoomFromId(roomId);
				if (temp_room)
				{
					Json::Value roomRsp;
					roomRsp["id"] = roomId;
					for (unsigned int i = 0; i < temp_room->sceneBleList.size(); i++)
					{
						Json::Value sceneRsp;
						sceneRsp["id"] = temp_room->sceneBleList[i]->GetId();
						sceneRsp["name"] = temp_room->sceneBleList[i]->GetName();
						roomRsp["scenes"].append(sceneRsp);
					}
					respValue["data"]["rooms"].append(roomRsp);
					respValue["data"]["code"] = CODE_OK;
				}
			}
		}
	}
	else
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	respValue["cmd"] = "getSceneIntoRoomRsp";
	return CODE_OK;
}

int Gateway::OnUpdateRoomName(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("name") && reqValue["name"].isString() && reqValue.isMember("id") && reqValue["id"].isString())
	{
		string id = reqValue["id"].asString();
		string name = reqValue["name"].asString();
		Room *room = getRoomFromId(id);
		if (room)
		{
			room->SetName(name);
			database->RoomUpdate(room, room->GetAddr());
		}
		Group *group = getGroupFromId(id);
		if (group)
		{
			group->SetName(name);
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "updateRoomNameRsp";
	return CODE_OK;
}
