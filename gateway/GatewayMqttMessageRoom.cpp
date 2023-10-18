#include "Gateway.h"
#include "Log.h"
#include "Db.h"
#include "DataSceneBle.h"

void Gateway::InitMqttMessageRoom()
{
	OnDeviceRpcCallbackRegister("getRoomList", bind(&Gateway::OnGetRoomList, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getDevListInRoom", bind(&Gateway::OnGetDevListInRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("createRoom", bind(&Gateway::OnCreateRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("addDevToRoom", bind(&Gateway::OnAddDeviceToRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("delDevToRoom", bind(&Gateway::OnDeleteDeviceFromRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("delRoom", bind(&Gateway::OnDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getGroupIntoRoom", bind(&Gateway::OnGetGroupIntoRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getSceneIntoRoom", bind(&Gateway::OnGetSceneIntoRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("updateRoomName", bind(&Gateway::OnUpdateRoomName, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("checkRoom", bind(&Gateway::OnCheckRoom, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("getRoomList", bind(&Gateway::OnGetRoomList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getDevListInRoom", bind(&Gateway::OnGetDevListInRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("createRoom", bind(&Gateway::OnCreateRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("addDevToRoom", bind(&Gateway::OnAddDeviceToRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delDevToRoom", bind(&Gateway::OnDeleteDeviceFromRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delRoom", bind(&Gateway::OnDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getGroupIntoRoom", bind(&Gateway::OnGetGroupIntoRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getSceneIntoRoom", bind(&Gateway::OnGetSceneIntoRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("updateRoomName", bind(&Gateway::OnUpdateRoomName, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("checkRoom", bind(&Gateway::OnCheckRoom, this, placeholders::_1, placeholders::_2));
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
	respValue["cmd"] = "createRoomRsp";
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("name") && reqValue["name"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray() &&
		reqValue.isMember("groups") && reqValue["groups"].isArray() &&
		reqValue.isMember("scenes") && reqValue["scenes"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string roomId = reqValue["id"].asString();
		string roomName = reqValue["name"].asString();
		Json::Value devicesValue = reqValue["devices"];
		Json::Value groupsValue = reqValue["groups"];
		Json::Value scenesValue = reqValue["scenes"];
		respValue["data"]["code"] = CODE_ERROR;

		map<string, bool> devicesStatusConfig;
		vector<Device *> devicesAddRoom;

		Room *room = getRoomFromId(roomId);
		if (!room)
		{
			uint16_t roomAddr = getNextRoomAddr();
			room = new Room(roomId, roomAddr, roomName);
			if (room)
			{
				gateway->AddNewRoom(room, true);
				for (auto &deviceValue : devicesValue)
				{
					if (deviceValue.isString())
					{
						string deviceId = deviceValue.asString();
						devicesStatusConfig[deviceId] = true;

						Device *device = getDeviceFromId(deviceId);
						if (device)
						{
							LOGW("device->GetVersion(): 0x%04X", device->GetVersion());
							devicesAddRoom.push_back(device);
							// TODO: get fast provisioning
							if (device->GetVersion() >= 0x0300)
							{
								if (room->AddDeviceOneMessage(device, true, true) != CODE_OK)
									devicesStatusConfig[deviceId] = false;
							}
							else
							{
								// Check lightBle send ble
								uint32_t type = device->GetType() / 10000;
								if (type == 1)
								{
									if (room->AddDevice(device, true, true) != CODE_OK)
										devicesStatusConfig[deviceId] = false;
								}
								else
								{
									if (room->AddDevice(device, false, true) != CODE_OK)
										devicesStatusConfig[deviceId] = false;
								}
							}
						}
						else
							LOGW("Device %s not found", deviceId.c_str());
					}
				}
				printRoom();
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
								group = new Group(id, roomAddr + Device::BleTypeToGroupId(type), name);
								if (group)
								{
									if (AddNewGroup(group, true))
										room->AddGroup(group, true, true);
									for (auto &deviceInRoom : room->deviceList)
									{
										if (deviceInRoom->device->GetType() == type)
										{
											if (deviceInRoom->device->GetVersion() < 0x0300)
											{
												if (group->AddDevice(deviceInRoom->device, deviceInRoom->device->GetAddr(), true, true) != CODE_OK)
													if (devicesStatusConfig[deviceInRoom->device->GetId()])
														devicesStatusConfig[deviceInRoom->device->GetId()] = false;
											}
											else
											{
												if (group->AddDevice(deviceInRoom->device, deviceInRoom->device->GetAddr(), false, true) != CODE_OK)
													if (devicesStatusConfig[deviceInRoom->device->GetId()])
														devicesStatusConfig[deviceInRoom->device->GetId()] = false;
											}
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

				printGroup();
				// for (auto &sceneValue : scenesValue)
				for (int i = 0; i < scenesValue.size(); i++)
				{
					Json::Value sceneValue = scenesValue[i];
					if (sceneValue.isObject())
					{
						if (sceneValue.isMember("id") && sceneValue["id"].isString() &&
							sceneValue.isMember("name") && sceneValue["name"].isString())
						{
							string id = sceneValue["id"].asString();
							string name = sceneValue["name"].asString();
							SceneBle *sceneBle = new SceneBle(id, roomAddr + i, name);
							if (sceneBle)
							{
								AddNewSceneBle(sceneBle, true);
								room->AddSceneBle(sceneBle, true, true);
								for (auto &deviceAddScene : devicesAddRoom)
								{
									if (deviceAddScene->GetVersion() < 0x0300)
									{
										if (sceneValue.isMember("groups") && sceneValue["groups"].isArray())
										{
											Json::Value groupsAddScene = sceneValue["groups"];
											for (auto &groupAddScene : groupsAddScene)
											{
												if (groupAddScene.isObject() && groupAddScene.isMember("id") && groupAddScene["id"].isString() &&
													groupAddScene.isMember("data") && groupAddScene["data"].isObject())
												{
													string groupId = groupAddScene["id"].asString();
													Json::Value groupData = groupAddScene["data"];
													Group *group = getGroupFromId(groupId);
													if (group)
													{
														if (group->GetPositionDevice(deviceAddScene, deviceAddScene->GetAddr()) >= 0)
														{
															group->Do(groupData, true);
															if (sceneBle->AddDevice(deviceAddScene, groupData, true, true) != CODE_OK)
																if (devicesStatusConfig[deviceAddScene->GetId()])
																	devicesStatusConfig[deviceAddScene->GetId()] = false;
														}
													}
												}
											}
										}
									}
									else
									{
										Json::Value dataScene = DataSceneBle::GetDataDeviceInScene(deviceAddScene->GetType(), i + 1);
										if (sceneBle->AddDevice(deviceAddScene, dataScene, false, true) != CODE_OK)
											if (devicesStatusConfig[deviceAddScene->GetId()])
												devicesStatusConfig[deviceAddScene->GetId()] = false;
									}
								}
							}
						}
					}
				}

				printScene();
				for (auto &[id, status] : devicesStatusConfig)
				{
					if (status)
						successList.append(id);
					else
						failedList.append(id);
				}
				respValue["data"]["code"] = CODE_OK;
				respValue["data"]["id"] = roomId;
				respValue["data"]["success"] = successList;
				respValue["data"]["failed"] = failedList;
				pushMsgHcCoreToHcApp("createRoom", roomId, roomName, successList);
			}
			else
			{
				respValue["data"]["code"] = CODE_MEMORY_ERROR;
			}

			if (room)
			{
				room->SetDataConfig(respValue.toString());
				database->RoomAdd(room);
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
	return CODE_OK;
}

int Gateway::OnAddDeviceToRoom(Json::Value &reqValue, Json::Value &respValue)
{
	respValue["cmd"] = "addDevToRoomRsp";
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string roomId = reqValue["id"].asString();
		Json::Value devicesValue = reqValue["devices"];

		map<string, bool> devicesStatusConfig;
		vector<Device *> devicesAddRoom;

		Room *room = getRoomFromId(roomId);
		if (room)
		{
			for (auto &deviceValue : devicesValue)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					devicesStatusConfig[deviceId] = true;
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						devicesAddRoom.push_back(device);
						if (device->GetVersion() >= 0x0300)
						{
							if (room->AddDeviceOneMessage(device, true, true) != CODE_OK)
								devicesStatusConfig[deviceId] = false;
						}
						else
						{
							uint32_t type = device->GetType() / 10000;
							if (type == 1)
							{
								if (room->AddDevice(device, true, true) != CODE_OK)
									devicesStatusConfig[deviceId] = false;
							}
							else
							{
								if (room->AddDevice(device, false, true) != CODE_OK)
									devicesStatusConfig[deviceId] = false;
							}
						}
					}
					else
					{
						devicesStatusConfig[deviceId] = false;
						LOGW("Device %s not found", deviceId.c_str());
					}
				}
			}
			printRoom();
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
									group = new Group(id, room->GetAddr() + Device::BleTypeToGroupId(type), name);
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
								for (auto &dev : devicesAddRoom)
								{
									if (dev->GetType() == type)
									{
										if (dev->GetVersion() < 0x0300)
										{
											if (group->AddDevice(dev, dev->GetAddr(), true, true) != CODE_OK)
												if (devicesStatusConfig[dev->GetId()])
													devicesStatusConfig[dev->GetId()] = false;
										}
										else
										{
											if (group->AddDevice(dev, dev->GetAddr(), false, true) != CODE_OK)
												if (devicesStatusConfig[dev->GetId()])
													devicesStatusConfig[dev->GetId()] = false;
										}
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
			printGroup();
			if (reqValue.isMember("scenes") && reqValue["scenes"].isArray())
			{
				Json::Value scenesValue = reqValue["scenes"];
				// for (auto &sceneValue : scenesValue)
				for (int i = 0; i < scenesValue.size(); i++)
				{
					Json::Value sceneValue = scenesValue[i];
					if (sceneValue.isObject())
					{
						if (sceneValue.isMember("id") && sceneValue["id"].isString())
						{
							string id = sceneValue["id"].asString();
							SceneBle *sceneBle = getSceneBleFromId(id);
							if (!sceneBle)
							{
								if (sceneValue.isMember("name") && sceneValue["name"].isString())
								{
									string name = sceneValue["name"].asString();
									sceneBle = new SceneBle(id, room->GetAddr() + i, name);
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
								for (auto &devInScene : devicesAddRoom)
								{
									if (devInScene->GetVersion() < 0x0300)
									{
										if (sceneValue.isMember("groups") && sceneValue["groups"].isArray())
										{
											Json::Value groupsAddScene = sceneValue["groups"];
											for (auto &groupAddScene : groupsAddScene)
											{
												if (groupAddScene.isObject() && groupAddScene.isMember("id") && groupAddScene["id"].isString() &&
													groupAddScene.isMember("data") && groupAddScene["data"].isObject())
												{
													string groupId = groupAddScene["id"].asString();
													Json::Value groupData = groupAddScene["data"];
													Group *group = getGroupFromId(groupId);
													if (group)
													{
														if (group->GetPositionDevice(devInScene, devInScene->GetAddr()) >= 0)
														{
															group->Do(groupData, true);
															if (sceneBle->AddDevice(devInScene, groupData, true, true) != CODE_OK)
																if (devicesStatusConfig[devInScene->GetId()])
																	devicesStatusConfig[devInScene->GetId()] = false;
														}
													}
												}
											}
										}
									}
									else
									{
										if (sceneBle->AddDevice(devInScene, DataSceneBle::GetDataDeviceInScene(devInScene->GetType(), i + 1), false, true) != CODE_OK)
											if (devicesStatusConfig[devInScene->GetId()])
												devicesStatusConfig[devInScene->GetId()] = false;
									}
								}
							}
						}
					}
				}
			}
			printScene();
			for (auto &[id, status] : devicesStatusConfig)
			{
				if (status)
					successList.append(id);
				else
					failedList.append(id);
			}
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
			pushMsgHcCoreToHcApp("addDevToRoom", roomId, room->GetName(), successList);

			if (room)
			{
				room->SetDataConfig(respValue.toString());
				database->RoomAdd(room);
			}
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
	return CODE_OK;
}

int Gateway::OnDeleteDeviceFromRoom(Json::Value &reqValue, Json::Value &respValue)
{
	respValue["cmd"] = "delDevFromRoomRsp";
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string roomId = reqValue["id"].asString();
		Json::Value devicesValue = reqValue["devices"];
		map<string, bool> devicesStatusConfig;
		Room *room = getRoomFromId(roomId);
		if (room)
		{
			for (auto &deviceValue : devicesValue)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					devicesStatusConfig[deviceId] = true;
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						if (device->GetVersion() >= 0x0300)
						{
							if (room->DelDeviceOneMessage(device, true, true) != CODE_OK)
								devicesStatusConfig[deviceId] = false;
						}
						else
						{
							if (room->DelDevice(device, true, true) != CODE_OK)
								devicesStatusConfig[deviceId] = false;
						}

						printRoom();

						for (auto &group : room->groupList)
						{
							for (auto &devInGr : group->deviceList)
							{
								if (devInGr->device->GetId() == device->GetId())
								{
									if (device->GetVersion() < 0x0300)
									{
										if (group->DelDevice(device, device->GetAddr(), true, true) != CODE_OK)
											if (devicesStatusConfig[device->GetId()])
												devicesStatusConfig[device->GetId()] = false;
									}
									else
									{
										if (group->DelDevice(device, device->GetAddr(), false, true) != CODE_OK)
											if (devicesStatusConfig[device->GetId()])
												devicesStatusConfig[device->GetId()] = false;
									}
								}
							}
						}

						printGroup();
						for (auto &sceneBle : room->sceneBleList)
						{
							for (auto &devInScene : sceneBle->deviceList)
							{
								if (devInScene->device->GetId() == device->GetId())
								{
									if (device->GetVersion() < 0x0300)
									{
										if (sceneBle->DelDevice(device, true, true) != CODE_OK)
											if (devicesStatusConfig[device->GetId()])
												devicesStatusConfig[device->GetId()] = false;
									}
									else
									{
										if (sceneBle->DelDevice(device, false, true) != CODE_OK)
											if (devicesStatusConfig[device->GetId()])
												devicesStatusConfig[device->GetId()] = false;
									}
								}
							}
						}
						printScene();
					}
					else
					{
						LOGD("deviceId %s dose not exist", deviceId.c_str());
						devicesStatusConfig[deviceId] = false;
					}
				}
			}
			for (auto &[id, status] : devicesStatusConfig)
			{
				if (status)
					successList.append(id);
				else
					failedList.append(id);
			}
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
			pushMsgHcCoreToHcApp("delDevFromRoom", roomId, room->GetName(), successList);

			if (room)
			{
				room->SetDataConfig(respValue.toString());
				database->RoomAdd(room);
			}
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_ROOM;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}

	return CODE_OK;
}

int Gateway::OnDeleteRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string roomId = reqValue["id"].asString();
		map<string, bool> devicesStatusConfig;
		Room *room = getRoomFromId(roomId);
		if (room)
		{
			for (auto &deviceInRoom : room->deviceList)
			{
				devicesStatusConfig[deviceInRoom->device->GetId()] = true;
			}

			vector<DeviceInGroup *> devInRoom = room->deviceList;
			for (auto &deviceInRoom : devInRoom)
			{
				if (deviceInRoom->device->GetVersion() >= 0x0300)
				{
					if (room->DelDeviceOneMessage(deviceInRoom->device, true, true) != CODE_OK)
						if (devicesStatusConfig[deviceInRoom->device->GetId()])
							devicesStatusConfig[deviceInRoom->device->GetId()] = false;
				}
				else
				{
					if (room->DelDevice(deviceInRoom->device, true, true) != CODE_OK)
						if (devicesStatusConfig[deviceInRoom->device->GetId()])
							devicesStatusConfig[deviceInRoom->device->GetId()] = false;
				}
			}
			devInRoom.clear();
			printRoom();
			// Delete all group
			vector<Group *> grpInRooms = room->groupList;
			for (auto &groupInRoom : grpInRooms)
			{
				vector<DeviceInGroup *> devInGroup = groupInRoom->deviceList;
				for (auto &deviceInGroup : devInGroup)
				{
					if (deviceInGroup->device->GetVersion() < 0x0300)
					{
						if (groupInRoom->DelDevice(deviceInGroup->device, deviceInGroup->device->GetAddr(), true, true) != CODE_OK)
							if (devicesStatusConfig[deviceInGroup->device->GetId()])
								devicesStatusConfig[deviceInGroup->device->GetId()] = false;
					}
					else
					{
						if (groupInRoom->DelDevice(deviceInGroup->device, deviceInGroup->device->GetAddr(), false, true) != CODE_OK)
							if (devicesStatusConfig[deviceInGroup->device->GetId()])
								devicesStatusConfig[deviceInGroup->device->GetId()] = false;
					}
				}
				delGroup(groupInRoom);
				devInGroup.clear();
			}
			printGroup();

			// Delete all scene
			vector<SceneBle *> sceneBleInRooms = room->sceneBleList;
			for (auto &sceneInRoom : sceneBleInRooms)
			{
				vector<DeviceInSceneBle *> devInSceneBle = sceneInRoom->deviceList;
				for (auto &deviceInScene : devInSceneBle)
				{
					if (deviceInScene->device->GetVersion() < 0x0300)
					{
						if (sceneInRoom->DelDevice(deviceInScene->device, true, true) != CODE_OK)
							if (devicesStatusConfig[deviceInScene->device->GetId()])
								devicesStatusConfig[deviceInScene->device->GetId()] = false;
					}
					else
					{
						if (sceneInRoom->DelDevice(deviceInScene->device, false, true) != CODE_OK)
							if (devicesStatusConfig[deviceInScene->device->GetId()])
								devicesStatusConfig[deviceInScene->device->GetId()] = false;
					}
				}
				delSceneBle(sceneInRoom);
				devInSceneBle.clear();
			}

			printScene();
			delRoom(room);

			for (auto &[id, status] : devicesStatusConfig)
			{
				if (status)
					successList.append(id);
				else
					failedList.append(id);
			}
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
			pushMsgHcCoreToHcApp("delDevFromRoom", roomId, room->GetName(), successList);

			printRoom();
			printGroup();
			printScene();
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

int Gateway::OnCheckRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string id = reqValue["id"].asString();
		Room *room = getRoomFromId(id);
		if (room)
		{
			Json::Value dataRsp;
			if (dataRsp.parse(room->GetDataConfig()) && dataRsp.isObject())
			{
				respValue = dataRsp;
				return CODE_OK;
			}
			else
				LOGW("Data error: %s", dataRsp.toString().c_str());
		}
		else
			LOGW("Room %s not found", id.c_str());
	}
	else
		LOGW("Data error: %s", reqValue.toString().c_str());

	return CODE_ERROR;
}
