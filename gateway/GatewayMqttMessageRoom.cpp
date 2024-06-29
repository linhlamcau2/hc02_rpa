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
	Json::Value roomData = Json::arrayValue;
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
		Json::Value roomsData = Json::arrayValue;
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
					Json::Value temp_devicesList = Json::arrayValue;
					vector<string> listDevId;
					bool isExist;
					for (unsigned int i = 0; i < temp_room->deviceList.size(); i++)
					{
						Json::Value device;
						DeviceInGroup *deviceInRoom = temp_room->deviceList[i];
						string deviceId = deviceInRoom->device->GetId();

						isExist = false;
						for (auto &id : listDevId)
						{
							if (deviceId == id)
							{
								isExist = true;
								break;
							}
						}

						if (!isExist)
						{
							listDevId.push_back(deviceId);
							Device *tempDev = getDeviceFromId(deviceId);
							device["id"] = deviceId;
							device["name"] = tempDev->GetName();
							device["type"] = (Json::Value::UInt)tempDev->GetType();
							temp_devicesList.append(device);
						}
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

int Gateway::CheckAddDevToRoom(Device *device, Room *room)
{
	int rs = CODE_OK;
	int devFast2Room = isDevFast2Room(device);
	if (devFast2Room == 0)
	{
		if (room->AddDeviceOneMessage(device, true, true) != CODE_OK)
			rs = CODE_ERROR;
	}
	else if (devFast2Room == 1)
	{
		if (room->AddDevice(device, true, true) != CODE_OK)
			rs = CODE_ERROR;
	}
	else if (devFast2Room == 2)
	{
		rs = CODE_ERROR;
		if (bleProtocol)
			if (bleProtocol->SetGroup(device->GetAddr(), room->GetAddr() + 49152) == CODE_OK)
				if (room->AddDevice(device, false, true) == CODE_OK)
					rs = CODE_OK;
	}
	else if (devFast2Room == -1)
	{
		if (room->AddDevice(device, false, true) != CODE_OK)
			rs = CODE_ERROR;
	}
	return rs;
}

int Gateway::AddDevToGroupInRoom(Device *device, Group *group, uint32_t type)
{
	int rs = CODE_OK;
	int indexTypeDevice = device->GetType() / 1000;
	int devFast2Room = isDevFast2Room(device);
	if (type == 1) // type nhóm công tắc
	{
		if (indexTypeDevice == 22 || indexTypeDevice == 24) // công tắc cảm ứng và công tắc cơ
		{
			if (devFast2Room == 0)
			{
				for (int i = 0; i < device->GetNumElement(); i++)
				{
					if (group->AddDevice(device, device->GetAddr() + i, false, true) != CODE_OK)
					{
						rs = CODE_ERROR;
					}
				}
			}
			else if (devFast2Room == 1)
			{
				for (int i = 0; i < device->GetNumElement(); i++)
				{
					if (group->AddDevice(device, device->GetAddr() + i, true, true) != CODE_OK)
					{
						rs = CODE_ERROR;
					}
				}
			}
		}
		else
		{
			rs = CODE_NOT_FOUND_DEVICE;
		}
	}
	else if (device->GetType() == type)
	{
		if (devFast2Room == 0)
		{
			if (group->AddDevice(device, device->GetAddr(), false, true) != CODE_OK)
			{
				rs = CODE_ERROR;
			}
		}
		else if (devFast2Room == 1)
		{
			if (group->AddDevice(device, device->GetAddr(), true, true) != CODE_OK)
			{
				rs = CODE_ERROR;
			}
		}
	}
	else
	{
		LOGW("Type not support");
		rs = CODE_NOT_FOUND_DEVICE;
	}

	return rs;
}

int Gateway::AddDevToSceneInRoom(Device *device, Json::Value &dataGroup, SceneBle *sceneBle, int indexSceneBle)
{
	int rs = CODE_OK;
	int indexTypeDevice = device->GetType() / 1000;
	int devFast2Room = isDevFast2Room(device);

	if (indexTypeDevice != 22 && indexTypeDevice != 24) // công tắc quét nhanh không có scene mặc định
	{
		if (devFast2Room == 0)
		{
			if (sceneBle->AddDevice(device, DataSceneBle::GetDataDeviceInScene(device->GetType(), indexSceneBle), false, true) != CODE_OK)
			{
				rs = CODE_ERROR;
			}
		}
		else if (devFast2Room == 1)
		{

			if (dataGroup.isObject() && dataGroup.isMember("id") && dataGroup["id"].isString() &&
				dataGroup.isMember("data") && dataGroup["data"].isObject())
			{
				string groupId = dataGroup["id"].asString();
				Json::Value groupData = dataGroup["data"];
				Group *group = getGroupFromId(groupId);
				if (group)
				{
					if (group->GetPositionDevice(device, device->GetAddr()) >= 0)
					{
						// group->Do(groupData, false);
						if (sceneBle->AddDevice(device, groupData, true, true) != CODE_OK)
						{
							rs = CODE_ERROR;
						}
					}
				}
			}
		}
	}

	return rs;
}

/**
 * Thêm thiết bị vào phòng
 * - Check type - version thiết bị để gửi thêm nhanh vào phòng
 * - Check type - version thiết bị để thêm vào nhóm thiết bị: nhóm đèn, nhóm công tắc, nhóm công tắc đèn
 * - Check type - version thiết bị để cấu hình scene
 */
int Gateway::DelDeviceFromAllRoom(string deviceId)
{
	for (const auto &[id, room] : roomList)
	{
		for (unsigned int i = 0; i < room->deviceList.size(); i++)
		{
			if (deviceId == room->deviceList[i]->device->GetId())
			{
				room->DelDevice(room->deviceList[i]->device, true, true);
			}
		}
	}
	return CODE_OK;
}

int Gateway::OnCreateRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("CreateRoom");
	respValue["cmd"] = "createRoomRsp";
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("name") && reqValue["name"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray() &&
		reqValue.isMember("groups") && reqValue["groups"].isArray() &&
		reqValue.isMember("scenes") && reqValue["scenes"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value objSuccessList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string roomId = reqValue["id"].asString();
		string roomName = reqValue["name"].asString();
		Json::Value devicesValue = reqValue["devices"];
		Json::Value groupsValue = reqValue["groups"];
		Json::Value scenesValue = reqValue["scenes"];
		respValue["data"]["code"] = CODE_ERROR;

		map<string, bool> devicesStatusConfig;
		vector<Device *> devicesAddRoom;
		vector<string> groupSceneSendtoHcApp;

		Room *room = getRoomFromId(roomId);
		if (!room)
		{
			uint16_t roomAddr = getNextRoomAddr();
			if (roomAddr > 0)
				room = new Room(roomId, roomAddr, roomName);
			if (room)
			{
				database->Sqlite_BenginTransaction();
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
							devicesAddRoom.push_back(device);
							if (CheckAddDevToRoom(device, room) != CODE_OK)
								devicesStatusConfig[deviceId] = false;
						}
						else
						{
							devicesStatusConfig[deviceId] = false;
							LOGW("Device %s not found", deviceId.c_str());
						}
						SLEEP_MS(100);
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
								Json::Value tempSuccessList = Json::arrayValue;
								group = new Group(id, roomAddr + Device::BleTypeToGroupId(type), name);
								if (group)
								{
									if (AddNewGroup(group, true))
										room->AddGroup(group, true, true);
									for (auto &deviceInRoom : devicesAddRoom)
									{
										int resultAddDevToGroup = AddDevToGroupInRoom(deviceInRoom, group, type);
										if (resultAddDevToGroup == CODE_ERROR)
										{
											if (devicesStatusConfig[deviceInRoom->GetId()])
											{
												devicesStatusConfig[deviceInRoom->GetId()] = false;
											}
										}
										else if (resultAddDevToGroup == CODE_OK)
										{
											if (devicesStatusConfig[deviceInRoom->GetId()])
											{
												tempSuccessList.append(deviceInRoom->GetId());
											}
										}
										SLEEP_MS(100);
									}
									groupSceneSendtoHcApp.push_back(CreateJsonGroupSceneSendHcCoreToHcApp("createGroup", group->GetId(), group->GetName(), tempSuccessList, roomId));
								}
								else
								{
									LOGW("createGroup error: id-%s, name-%s, type-%d", id.c_str(), name.c_str(), type);
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
								Json::Value tempSuccessList = Json::arrayValue;
								AddNewSceneBle(sceneBle, true);
								room->AddSceneBle(sceneBle, true, true);
								// for (auto &deviceAddScene : devicesAddRoom)
								// {
								if (sceneValue.isMember("groups") && sceneValue["groups"].isArray())
								{
									Json::Value groupsAddScene = sceneValue["groups"];
									for (auto &groupAddScene : groupsAddScene)
									{
										if (groupAddScene.isObject() &&
											groupAddScene.isMember("id") && groupAddScene["id"].isString() &&
											groupAddScene.isMember("data") && groupAddScene["data"].isObject())
										{
											string grpId = groupAddScene["id"].asString();
											Group *grp = getGroupFromId(grpId);
											if (grp)
											{
												grp->Do(groupAddScene["data"], false);
												for (auto &deviceAddScene : devicesAddRoom)
												{
													if (AddDevToSceneInRoom(deviceAddScene, groupAddScene, sceneBle, i + 1) != CODE_OK)
													{
														if (devicesStatusConfig[deviceAddScene->GetId()])
														{
															devicesStatusConfig[deviceAddScene->GetId()] = false;
														}
													}
													else
													{
														if (devicesStatusConfig[deviceAddScene->GetId()])
														{
															tempSuccessList.append(deviceAddScene->GetId());
														}
													}
												}
											}
										}
									}
									SLEEP_MS(100);
								}
								// }
								groupSceneSendtoHcApp.push_back(CreateJsonGroupSceneSendHcCoreToHcApp("createScene", sceneBle->GetId(), sceneBle->GetName(), tempSuccessList, roomId));
							}
						}
					}
				}

				printScene();
				for (auto &[id, status] : devicesStatusConfig)
				{
					if (status)
					{
						Json::Value temp;
						temp["id"] = id;
						Device *tempdv = getDeviceFromId(id);
						temp["name"] = tempdv->GetName();
						temp["type"] = (Json::Value::UInt)tempdv->GetType();
						objSuccessList.append(temp);
						successList.append(id);
					}
					else
						failedList.append(id);
				}
				respValue["data"]["code"] = CODE_OK;
				respValue["data"]["id"] = roomId;
				respValue["data"]["success"] = successList;
				respValue["data"]["failed"] = failedList;
				pushMsgHcCoreToHcApp("createRoom", roomId, roomName, objSuccessList, "");
				for (int i = 0; i < groupSceneSendtoHcApp.size(); i++)
				{
					PublishToLocalMessage(groupSceneSendtoHcApp[i]);
				}
				database->Sqlite_EndTransaction();
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
		Json::Value objSuccessList = Json::arrayValue;

		map<string, bool> devicesStatusConfig;
		vector<Device *> devicesAddRoom;
		vector<string> groupSceneSendtoHcApp;

		Room *room = getRoomFromId(roomId);
		if (room)
		{
			database->Sqlite_BenginTransaction();
			for (auto &deviceValue : devicesValue)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					devicesStatusConfig[deviceId] = true;
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						DelDeviceFromAllRoom(deviceId);
						devicesAddRoom.push_back(device);
						if (CheckAddDevToRoom(device, room) != CODE_OK)
							devicesStatusConfig[deviceId] = false;
					}
					else
					{
						devicesStatusConfig[deviceId] = false;
						LOGW("Device %s not found", deviceId.c_str());
					}
					SLEEP_MS(100);
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
							Json::Value tempSuccessList = Json::arrayValue;
							string cmd = "addDevToGroup";
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
										{
											room->AddGroup(group, true, true);
											cmd = "createGroup";
										}
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
									int resultAddDevToGroup = AddDevToGroupInRoom(dev, group, type);
									if (resultAddDevToGroup == CODE_ERROR)
									{
										if (devicesStatusConfig[dev->GetId()])
										{
											devicesStatusConfig[dev->GetId()] = false;
										}
									}
									else if (resultAddDevToGroup == CODE_OK)
									{
										if (devicesStatusConfig[dev->GetId()])
										{
											tempSuccessList.append(dev->GetId());
										}
									}
									SLEEP_MS(100);
								}
								groupSceneSendtoHcApp.push_back(CreateJsonGroupSceneSendHcCoreToHcApp(cmd, group->GetId(), group->GetName(), tempSuccessList, roomId));
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
							Json::Value tempSuccessList = Json::arrayValue;
							string cmd = "addDevToScene";
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
										cmd = "createScene";
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
								// for (auto &devInScene : devicesAddRoom)
								// {
								if (sceneValue.isMember("groups") && sceneValue["groups"].isArray())
								{
									Json::Value groupsAddScene = sceneValue["groups"];
									for (auto &groupAddScene : groupsAddScene)
									{
										if (groupAddScene.isObject() &&
											groupAddScene.isMember("id") && groupAddScene["id"].isString() &&
											groupAddScene.isMember("data") && groupAddScene["data"].isObject())
										{
											string grpId = groupAddScene["id"].asString();
											Group *grp = getGroupFromId(grpId);
											if (grp)
											{
												grp->Do(groupAddScene["data"], false);
												for (auto &devInScene : devicesAddRoom)
												{
													if (AddDevToSceneInRoom(devInScene, groupAddScene, sceneBle, i + 1) != CODE_OK)
													{
														if (devicesStatusConfig[devInScene->GetId()])
														{
															devicesStatusConfig[devInScene->GetId()] = false;
														}
													}
													else
													{
														if (devicesStatusConfig[devInScene->GetId()])
														{
															tempSuccessList.append(devInScene->GetId());
														}
													}
												}
											}
										}
									}
									SLEEP_MS(100);
								}
								// }
								groupSceneSendtoHcApp.push_back(CreateJsonGroupSceneSendHcCoreToHcApp(cmd, sceneBle->GetId(), sceneBle->GetName(), tempSuccessList, roomId));
							}
						}
					}
				}
			}
			printScene();
			for (auto &[id, status] : devicesStatusConfig)
			{
				if (status)
				{
					Json::Value temp;
					temp["id"] = id;
					Device *tempdv = getDeviceFromId(id);
					temp["name"] = tempdv->GetName();
					temp["type"] = (Json::Value::UInt)tempdv->GetType();
					objSuccessList.append(temp);
					successList.append(id);
				}
				else
					failedList.append(id);
			}
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
			pushMsgHcCoreToHcApp("addDevToRoom", roomId, room->GetName(), objSuccessList, "");
			for (int i = 0; i < groupSceneSendtoHcApp.size(); i++)
			{
				PublishToLocalMessage(groupSceneSendtoHcApp[i]);
			}
			if (room)
			{
				room->SetDataConfig(respValue.toString());
				database->RoomAdd(room);
			}

			database->Sqlite_EndTransaction();
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
			database->Sqlite_BenginTransaction();
			for (auto &deviceValue : devicesValue)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					devicesStatusConfig[deviceId] = true;
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						int devFast2Room = isDevFast2Room(device);
						if (devFast2Room == 0)
						{
							if (room->DelDeviceOneMessage(device, true, true) != CODE_OK)
								devicesStatusConfig[deviceId] = false;
						}
						else if (devFast2Room == 1)
						{
							if (room->DelDevice(device, true, true) != CODE_OK)
								devicesStatusConfig[deviceId] = false;
						}
						else if (devFast2Room == 2)
						{
							if (room->DelDevice(device, false, true) != CODE_OK)
								devicesStatusConfig[deviceId] = false;
						}

						printRoom();

						for (auto &group : room->groupList)
						{
							Json::Value tempSuccessList = Json::arrayValue;
							vector<DeviceInGroup *> devInGrpTemp = group->deviceList;
							for (auto &devInGr : devInGrpTemp)
							{
								if (devInGr->device->GetId() == device->GetId())
								{
									if (devFast2Room == 1)
									{
										if (group->DelDevice(devInGr->device, devInGr->epId, true, true) != CODE_OK)
										{
											if (devicesStatusConfig[device->GetId()])
												devicesStatusConfig[device->GetId()] = false;
										}
										else
										{
											tempSuccessList.append(device->GetId());
										}
									}
									else if (devFast2Room == 0)
									{
										bool isSendBle = false;
										if (group->GetAddr() < ROOM_ADDR_START)
											isSendBle = true;
										if (group->DelDevice(devInGr->device, devInGr->epId, isSendBle, true) != CODE_OK)
										{
											if (devicesStatusConfig[device->GetId()])
												devicesStatusConfig[device->GetId()] = false;
										}
										else
										{
											tempSuccessList.append(device->GetId());
										}
									}
									SLEEP_MS(100);
								}
							}
							pushMsgHcCoreToHcApp("delDevFromGroup", group->GetId(), group->GetName(), tempSuccessList, "");
						}

						printGroup();
						for (auto &sceneBle : room->sceneBleList)
						{
							Json::Value tempSuccessList = Json::arrayValue;
							vector<DeviceInSceneBle *> devInSceneTemp = sceneBle->deviceList;
							for (auto &devInScene : devInSceneTemp)
							{
								if (devInScene->device->GetId() == device->GetId())
								{
									if (devFast2Room == 1)
									{
										if (sceneBle->DelDevice(device, true, true) != CODE_OK)
										{
											if (devicesStatusConfig[device->GetId()])
												devicesStatusConfig[device->GetId()] = false;
										}
										else
										{
											tempSuccessList.append(deviceId);
										}
									}
									else if (devFast2Room == 0)
									{
										bool isSendBle = false;
										if (sceneBle->GetAddr() < ROOM_ADDR_START)
											isSendBle = true;
										if (sceneBle->DelDevice(device, isSendBle, true) != CODE_OK)
										{
											if (devicesStatusConfig[device->GetId()])
												devicesStatusConfig[device->GetId()] = false;
										}
										else
										{
											tempSuccessList.append(deviceId);
										}
									}
									SLEEP_MS(100);
								}
							}
							pushMsgHcCoreToHcApp("delDevToScene", sceneBle->GetId(), sceneBle->GetName(), tempSuccessList, "");
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
			pushMsgHcCoreToHcApp("delDevFromRoom", roomId, room->GetName(), successList, "");

			database->Sqlite_EndTransaction();

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
			database->Sqlite_BenginTransaction();
			for (auto &deviceInRoom : room->deviceList)
			{
				devicesStatusConfig[deviceInRoom->device->GetId()] = true;
			}

			vector<DeviceInGroup *> devInRoom = room->deviceList;
			for (auto &deviceInRoom : devInRoom)
			{
				int devFast2Room = isDevFast2Room(deviceInRoom->device);
				if (devFast2Room == 0)
				{
					if (room->DelDeviceOneMessage(deviceInRoom->device, true, true) != CODE_OK)
						if (devicesStatusConfig[deviceInRoom->device->GetId()])
							devicesStatusConfig[deviceInRoom->device->GetId()] = false;
				}
				else if (devFast2Room == 1)
				{
					if (room->DelDevice(deviceInRoom->device, true, true) != CODE_OK)
						if (devicesStatusConfig[deviceInRoom->device->GetId()])
							devicesStatusConfig[deviceInRoom->device->GetId()] = false;
				}
				else if (devFast2Room == 2)
				{
					if (room->DelDevice(deviceInRoom->device, false, true) != CODE_OK)
						if (devicesStatusConfig[deviceInRoom->device->GetId()])
							devicesStatusConfig[deviceInRoom->device->GetId()] = false;
				}
				SLEEP_MS(100);
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
					int devGroupFast2Room = isDevFast2Room(deviceInGroup->device);
					if (devGroupFast2Room == 1)
					{
						if (groupInRoom->DelDevice(deviceInGroup->device, deviceInGroup->epId, true, true) != CODE_OK)
							if (devicesStatusConfig[deviceInGroup->device->GetId()])
								devicesStatusConfig[deviceInGroup->device->GetId()] = false;
					}
					else if (devGroupFast2Room == 0)
					{
						bool isSendBle = false;
						if (groupInRoom->GetAddr() < ROOM_ADDR_START)
							isSendBle = true;
						if (groupInRoom->DelDevice(deviceInGroup->device, deviceInGroup->epId, isSendBle, true) != CODE_OK)
							if (devicesStatusConfig[deviceInGroup->device->GetId()])
								devicesStatusConfig[deviceInGroup->device->GetId()] = false;
					}
					SLEEP_MS(100);
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
					int devSceneFast2Room = isDevFast2Room(deviceInScene->device);
					if (devSceneFast2Room == 1)
					{
						if (sceneInRoom->DelDevice(deviceInScene->device, true, true) != CODE_OK)
							if (devicesStatusConfig[deviceInScene->device->GetId()])
								devicesStatusConfig[deviceInScene->device->GetId()] = false;
					}
					else if (devSceneFast2Room == 0)
					{
						bool isSendBle = false;
						if (sceneInRoom->GetAddr() < ROOM_ADDR_START)
							isSendBle = true;
						if (sceneInRoom->DelDevice(deviceInScene->device, isSendBle, true) != CODE_OK)
							if (devicesStatusConfig[deviceInScene->device->GetId()])
								devicesStatusConfig[deviceInScene->device->GetId()] = false;
					}
					SLEEP_MS(100);
				}
				delSceneBle(sceneInRoom);
				devInSceneBle.clear();
			}

			printScene();

			vector<Rule *> ruleInRooms = room->ruleList;
			for (auto &ruleInRoom : ruleInRooms)
			{
				Json::Value temp = Json::arrayValue;
				pushMsgHcCoreToHcApp("delRule", ruleInRoom->GetId(), ruleInRoom->GetName(), temp, "");
				delRule(ruleInRoom);
			}

			for (auto &[id, status] : devicesStatusConfig)
			{
				if (status)
					successList.append(id);
				else
					failedList.append(id);
			}

			pushMsgHcCoreToHcApp("delRoom", roomId, room->GetName(), successList, "");
			delRoom(room);

			database->Sqlite_EndTransaction();

			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;

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
			database->RoomUpdate(room);
		}
		Group *group = getGroupFromId(id);
		if (group)
		{
			group->SetName(name);
		}
	}

	Json::Value dataPushToHcApp;
	dataPushToHcApp["cmd"] = "updateRoomName";
	dataPushToHcApp["data"] = reqValue;
	PublishToLocalMessage(dataPushToHcApp);
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
