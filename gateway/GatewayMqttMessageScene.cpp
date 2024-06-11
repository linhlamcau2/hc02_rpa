#include "Gateway.h"
#include "Log.h"
#include "Db.h"
#include "DeviceBleSeftPowerRemote.h"

void Gateway::InitMqttMessageScene()
{
	// OnDeviceRpcCallbackRegister("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("createScene", bind(&Gateway::OnCreateScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("editScene", bind(&Gateway::OnEditScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delScene", bind(&Gateway::OnDeleteScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("callScene", bind(&Gateway::OnCallScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("createSceneController", bind(&Gateway::OnCreateSceneController, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delSceneController", bind(&Gateway::OnDelSceneController, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getSceneList", bind(&Gateway::OnGetSceneList, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getDevListInScene", bind(&Gateway::OnGetDevListInScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("addFavoriteScene", bind(&Gateway::OnAddFavoriteScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delFavoriteScene", bind(&Gateway::OnDelFavoriteScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getFavoriteScene", bind(&Gateway::OnGetFavoriteScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("updateSceneName", bind(&Gateway::OnUpdateSceneName, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("addDevToScene", bind(&Gateway::OnAddDevToScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delDevToScene", bind(&Gateway::OnDelDevToScene, this, placeholders::_1, placeholders::_2));

	// OnLocalCallbackRegister("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("createScene", bind(&Gateway::OnCreateScene, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("editScene", bind(&Gateway::OnEditScene, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delScene", bind(&Gateway::OnDeleteScene, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("callScene", bind(&Gateway::OnCallScene, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("createSceneController", bind(&Gateway::OnCreateSceneController, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delSceneController", bind(&Gateway::OnDelSceneController, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getSceneList", bind(&Gateway::OnGetSceneList, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getDevListInScene", bind(&Gateway::OnGetDevListInScene, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("addFavoriteScene", bind(&Gateway::OnAddFavoriteScene, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delFavoriteScene", bind(&Gateway::OnDelFavoriteScene, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getFavoriteScene", bind(&Gateway::OnGetFavoriteScene, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("updateSceneName", bind(&Gateway::OnUpdateSceneName, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("addDevToScene", bind(&Gateway::OnAddDevToScene, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delDevToScene", bind(&Gateway::OnDelDevToScene, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnControlScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlScene");
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string sceneId = reqValue["id"].asString();
		SceneBle *sceneBle = getSceneBleFromId(sceneId);
		if (sceneBle)
		{
			int rs = CODE_OK;
			if (sceneBle->deviceList.size() <= 20)
				rs = sceneBle->Do(true);
			else
			{
				rs = sceneBle->Do(false);
				Json::Value devicesData = Json::arrayValue;
				for (auto &devInSceneBle : sceneBle->deviceList)
				{
					if (devInSceneBle->device->isOnline())
					{
						Json::Value deviceData;
						deviceData["id"] = devInSceneBle->device->GetId();
						deviceData["data"] = devInSceneBle->data;
						devicesData.append(deviceData);
					}
				}
				Json::Value dataPush;
				dataPush["device"] = devicesData;
				gateway->pushDeviceUpdateLocal(dataPush);
				gateway->pushDeviceUpdateCloud(dataPush);
			}
			respValue["data"]["code"] = rs;
		}
		else
		{
			LOGW("Scene %s dose not exsit", sceneId.c_str());
			respValue["data"]["code"] = CODE_NOT_FOUND_SCENE;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlScene %s format error", reqValue.toString().c_str());
	}
	Json::Value dataPushToHcApp;
	dataPushToHcApp["cmd"] = "controlScene";
	dataPushToHcApp["data"] = reqValue;
	PublishToLocalMessage(dataPushToHcApp);
	respValue["cmd"] = "controlSceneRsp";
	return CODE_OK;
}

int Gateway::OnGetSceneList(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetSceneList");
	Json::Value sceneData = Json::arrayValue;
	sceneBleListMtx.lock();
	for (const auto &[id, scene] : sceneBleList)
	{
		Json::Value sceneValue;
		sceneValue["id"] = scene->GetId();
		sceneValue["name"] = scene->GetName();
		sceneData.append(sceneValue);
	}
	sceneBleListMtx.unlock();
	respValue["data"]["scenes"] = sceneData;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getSceneListRsp";
	return CODE_OK;
}

int Gateway::OnGetDevListInScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetDevListInScene");
	if (reqValue.isMember("scenes") && reqValue["scenes"].isArray() && reqValue["scenes"].size() > 0)
	{
		Json::Value scenesList = Json::arrayValue;
		Json::Value scenes = reqValue["scenes"];
		for (auto &sceneValue : scenes)
		{
			Json::Value scenesData = Json::objectValue;
			if (sceneValue.isString())
			{
				string sceneId = sceneValue.asString();
				scenesData["id"] = sceneId;
				SceneBle *temp_scene = getSceneBleFromId(sceneId);
				if (temp_scene)
				{
					Json::Value temp_devicesList = Json::arrayValue;
					for (unsigned int i = 0; i < temp_scene->deviceList.size(); i++)
					{
						DeviceInSceneBle *deviceInSceneBle = temp_scene->deviceList[i];
						string deviceId = deviceInSceneBle->device->GetId();
						temp_devicesList.append(deviceId);
					}
					scenesData["devices"] = temp_devicesList;
				}
				else
					respValue["data"]["code"] = CODE_NOT_FOUND_ROOM;
			}
			else
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
			scenesList.append(scenesData);
		}
		respValue["data"]["scenes"] = scenesList;
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getDevListInScene";
	return CODE_OK;
}

int Gateway::OnCreateScene(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("name") && reqValue["name"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string sceneId = reqValue["id"].asString();
		string sceneName = reqValue["name"].asString();
		// TODO: add start address of normal scene
		int sceneAddr = getNextSceneBleAddr();
		SceneBle *sceneBle = new SceneBle(sceneId, sceneAddr, sceneName);
		if (sceneBle)
		{
			if (AddNewSceneBle(sceneBle, true))
			{
				string roomId;
				Json::Value deviceList = reqValue["devices"];
				database->Sqlite_BenginTransaction();
				for (auto &deviceValue : deviceList)
				{
					if (deviceValue.isObject() &&
						deviceValue.isMember("id") && deviceValue["id"].isString() &&
						deviceValue.isMember("data") && deviceValue["data"].isObject())
					{
						Json::Value deviceProperties = deviceValue["data"];
						string deviceId = deviceValue["id"].asString();
						Device *device = getDeviceFromId(deviceId);
						if (device)
						{
							if (sceneBle->AddDevice(device, deviceProperties, true, true) == CODE_OK)
							{
								// database->DeviceInSceneBleAdd(sceneBle, device, deviceProperties.toString());
								successList.append(device->GetId());
							}
							else
							{
								failedList.append(device->GetId());
							}
						}
						SLEEP_MS(100);
					}
				}
				database->Sqlite_EndTransaction();
				if (reqValue.isMember("roomId") && reqValue["roomId"].isString())
				{
					roomId = reqValue["roomId"].asString();
					Room *room = getRoomFromId(roomId);
					if (room)
					{
						room->AddSceneBle(sceneBle, true, true);
					}
				}
				respValue["data"]["code"] = CODE_OK;
				respValue["data"]["id"] = sceneId;
				respValue["data"]["addr"] = sceneAddr;
				respValue["data"]["success"] = successList;
				respValue["data"]["failed"] = failedList;
				pushMsgHcCoreToHcApp("createScene", sceneId, sceneBle->GetName(), successList, roomId);

				printScene();
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
	respValue["cmd"] = "createSceneRsp";
	return CODE_OK;
}

int Gateway::OnEditScene(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("name") && reqValue["name"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string sceneId = reqValue["id"].asString();
		string sceneName = reqValue["name"].asString();
		SceneBle *sceneBle = getSceneBleFromId(sceneId);
		if (!sceneBle)
		{
			Rule *rule = getRuleFromId(sceneId);
			if (rule)
			{
				delRule(rule);
			}
			int sceneAddr = getNextSceneBleAddr();
			sceneBle = new SceneBle(sceneId, sceneAddr, sceneName);
			if (sceneBle)
				AddNewSceneBle(sceneBle, true);
		}
		if (sceneBle)
		{
			database->Sqlite_BenginTransaction();
			vector<Device *> devsInScene; // list dev in scene
			for (auto &devs : sceneBle->deviceList)
			{
				devsInScene.push_back(devs->device);
			}
			vector<Device *> listDevicesDel; // list old dev del scene
			vector<Device *> devsEditScene;	 // list dev edit scene

			Json::Value deviceList = reqValue["devices"];
			map<Device *, Json::Value> listData;
			for (auto &deviceValue : deviceList)
			{
				if (deviceValue.isObject() &&
					deviceValue.isMember("id") && deviceValue["id"].isString() &&
					deviceValue.isMember("data") && deviceValue["data"].isObject())
				{
					Json::Value deviceProperties = deviceValue["data"];
					string deviceId = deviceValue["id"].asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						listData[device] = deviceProperties;
						devsEditScene.push_back(device);
					}
				}
			}

			// Find list device del scene
			for (auto &item : devsInScene)
			{
				if (find(devsEditScene.begin(), devsEditScene.end(), item) == devsEditScene.end())
				{
					listDevicesDel.push_back(item);
				}
			}

			// Phai xoa list device delscene truoc
			for (auto &item : listDevicesDel)
			{
				if (sceneBle->DelDevice(item, true, true) == CODE_OK)
				{
					successList.append(item->GetId());
				}
				else
				{
					failedList.append(item->GetId());
				}
				SLEEP_MS(100);
			}

			for (auto &item : devsEditScene)
			{
				// Neu la congtac xoa het scene cua cac nut, xong them lai
				if ((item->GetType() / 1000) == 22 || (item->GetType() / 1000) == 24)
				{
					sceneBle->DelDevice(item, true, true);
				}

				if (sceneBle->AddDevice(item, listData[item], true, true) == CODE_OK)
				{
					successList.append(item->GetId());
				}
				else
				{
					failedList.append(item->GetId());
				}
				SLEEP_MS(100);
			}

			if (reqValue.isMember("roomId") && reqValue["roomId"].isString())
			{
				string roomId = reqValue["roomId"].asString();
				Room *room = getRoomFromId(roomId);
				if (room)
				{
					room->AddSceneBle(sceneBle, true, true);
				}
			}
			database->Sqlite_EndTransaction();
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["id"] = sceneId;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
			pushMsgHcCoreToHcApp("editScene", sceneId, sceneBle->GetName(), successList, "");

			printScene();
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_SCENE;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "editSceneRsp";
	return CODE_OK;
}

int Gateway::OnDeleteScene(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string sceneId = reqValue["id"].asString();
		SceneBle *sceneBle = getSceneBleFromId(sceneId);
		if (sceneBle)
		{
			database->Sqlite_BenginTransaction();
			vector<DeviceInSceneBle *> devicesInSceneBle = sceneBle->deviceList;
			for (auto &deviceInScene : devicesInSceneBle)
			{
				if (sceneBle->DelDevice(deviceInScene->device, true, true) == CODE_OK)
				{
					// database->DeviceInSceneBleDel(sceneBle, deviceInScene->device);
					successList.append(deviceInScene->device->GetId());
				}
				else
				{
					failedList.append(deviceInScene->device->GetId());
				}
				SLEEP_MS(100);
			}
			database->Sqlite_EndTransaction();
			pushMsgHcCoreToHcApp("delScene", sceneId, sceneBle->GetName(), successList, "");

			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["id"] = sceneId;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
			delSceneBle(sceneBle);
			printScene();
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_SCENE;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "delSceneRsp";
	return CODE_OK;
}

int Gateway::OnCallScene(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string sceneId = reqValue["id"].asString();
		SceneBle *sceneBle = getSceneBleFromId(sceneId);
		if (sceneBle)
		{
			sceneBle->Do();
			respValue["data"]["code"] = CODE_OK;
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_SCENE;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "callSceneRsp";
	return CODE_OK;
}

int Gateway::OnAddDevToScene(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("name") && reqValue["name"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string sceneId = reqValue["id"].asString();
		string sceneName = reqValue["name"].asString();
		SceneBle *sceneBle = getSceneBleFromId(sceneId);
		if (sceneBle)
		{
			database->Sqlite_BenginTransaction();
			Json::Value deviceList = reqValue["devices"];
			for (auto &deviceValue : deviceList)
			{
				if (deviceValue.isObject() &&
					deviceValue.isMember("id") && deviceValue["id"].isString() &&
					deviceValue.isMember("data") && deviceValue["data"].isObject())
				{
					Json::Value deviceProperties = deviceValue["data"];
					string deviceId = deviceValue["id"].asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						if (sceneBle->AddDevice(device, deviceProperties, true, true) == CODE_OK)
						{
							successList.append(deviceId);
						}
						else
						{
							failedList.append(deviceId);
						}
					}
				}
				SLEEP_MS(100);
			}
			database->Sqlite_EndTransaction();

			if (reqValue.isMember("roomId") && reqValue["roomId"].isString())
			{
				string roomId = reqValue["roomId"].asString();
				Room *room = getRoomFromId(roomId);
				if (room)
				{
					room->AddSceneBle(sceneBle, true, true);
				}
			}
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["id"] = sceneId;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
			pushMsgHcCoreToHcApp("addDevToScene", sceneId, sceneBle->GetName(), successList, "");

			printScene();
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_SCENE;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "addDevToSceneRsp";
	return CODE_OK;
}
int Gateway::OnDelDevToScene(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("name") && reqValue["name"].isString() &&
		reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		string sceneId = reqValue["id"].asString();
		string sceneName = reqValue["name"].asString();
		SceneBle *sceneBle = getSceneBleFromId(sceneId);
		if (sceneBle)
		{
			database->Sqlite_BenginTransaction();
			Json::Value deviceList = reqValue["devices"];
			for (auto &deviceValue : deviceList)
			{
				if (deviceValue.isObject() &&
					deviceValue.isMember("id") && deviceValue["id"].isString() &&
					deviceValue.isMember("data") && deviceValue["data"].isObject())
				{
					Json::Value deviceProperties = deviceValue["data"];
					string deviceId = deviceValue["id"].asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						if (sceneBle->DelDevice(device, true, true) == CODE_OK)
						{
							successList.append(deviceId);
						}
						else
						{
							failedList.append(deviceId);
						}
					}
					SLEEP_MS(100);
				}
			}
			database->Sqlite_EndTransaction();

			if (reqValue.isMember("roomId") && reqValue["roomId"].isString())
			{
				string roomId = reqValue["roomId"].asString();
				Room *room = getRoomFromId(roomId);
				if (room)
				{
					room->AddSceneBle(sceneBle, true, true);
				}
			}
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["id"] = sceneId;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
			pushMsgHcCoreToHcApp("delDevToScene", sceneId, sceneBle->GetName(), successList, "");

			printScene();
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_SCENE;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "delDevToSceneRsp";
	return CODE_OK;
}

int Gateway::ConfigSceneForRemote(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene)
{
	if (!device)
		return CODE_ERROR;

	if (data.isArray() && scene.isObject())
	{
		int result = CODE_OK;

		if (scene.isMember("id") && scene["id"].isString())
		{
			string sceneBleId = scene["id"].asString();
			SceneBle *sceneBle = getSceneBleFromId(sceneBleId);
			if (sceneBle)
			{
				uint16_t convertButton = 0;
				uint8_t bt1 = 0;
				uint8_t bt2 = 0;
				uint8_t bt3 = 0;
				uint8_t bt4 = 0;
				int sizeData = data.size();
				for (auto &dt : data)
				{
					if (dt.isObject())
					{
						if (device->GetType() == BLE_DC_SCENE_CONTACT || device->GetType() == BLE_REMOTE_M3 || device->GetType() == BLE_REMOTE_M3_V2 || device->GetType() == BLE_REMOTE_M4)
						{
							if (isAddScene)
							{
								if (dt.isMember("bt") && dt["bt"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 1, dt["bt"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt2") && dt["bt2"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 2, dt["bt2"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt3") && dt["bt3"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 3, dt["bt3"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt4") && dt["bt4"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 4, dt["bt4"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt5") && dt["bt5"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 5, dt["bt5"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt6") && dt["bt6"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 6, dt["bt6"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
							}
							else
							{
								if (dt.isMember("bt") && dt["bt"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 1, dt["bt"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt2") && dt["bt2"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 2, dt["bt2"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt3") && dt["bt3"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 3, dt["bt3"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt4") && dt["bt4"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 4, dt["bt4"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt5") && dt["bt5"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 5, dt["bt5"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt6") && dt["bt6"].isInt())
									if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), 6, dt["bt6"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
							}
						}

						if (device->GetType() == BLE_AC_SCENE_CONTACT || device->GetType() == BLE_AC_SCENE_CONTACT_RGB || device->GetType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE)
						{
							if (isAddScene)
							{
								if (dt.isMember("bt") && dt["bt"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 1, dt["bt"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt2") && dt["bt2"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 2, dt["bt2"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt3") && dt["bt3"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 3, dt["bt3"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt4") && dt["bt4"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 4, dt["bt4"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt5") && dt["bt5"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 5, dt["bt5"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt6") && dt["bt6"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 6, dt["bt6"].asInt(), sceneBle->GetAddr(), 0) != CODE_OK)
										result = CODE_ERROR;
							}
							else
							{
								if (dt.isMember("bt") && dt["bt"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 1, dt["bt"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt2") && dt["bt2"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 2, dt["bt2"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt3") && dt["bt3"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 3, dt["bt3"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt4") && dt["bt4"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 4, dt["bt4"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt5") && dt["bt5"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 5, dt["bt5"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
								if (dt.isMember("bt6") && dt["bt6"].isInt())
									if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), 6, dt["bt6"].asInt(), 0, 0) != CODE_OK)
										result = CODE_ERROR;
							}
						}
						if (device->GetType() == BLE_SEFTPOWER_REMOTE_1 || device->GetType() == BLE_SEFTPOWER_REMOTE_2 || device->GetType() == BLE_SEFTPOWER_REMOTE_3)
						{
							if (dt.isMember("bt") && dt["bt"].isInt())
								bt1 = 1;
							else if (dt.isMember("bt2") && dt["bt2"].isInt())
								bt2 = 1;
							else if (dt.isMember("bt3") && dt["bt3"].isInt())
								bt3 = 1;
							else if (dt.isMember("bt4") && dt["bt4"].isInt())
								bt4 = 1;
							if (sizeData == 1)
							{
								convertButton = bt1 + bt2 * 2 + bt3 * 4 + bt4 * 8;
								DeviceBleSeftPowerRemote *deviceBleSeftPowerRemote = dynamic_cast<DeviceBleSeftPowerRemote *>(device);
								if (deviceBleSeftPowerRemote)
								{
									Device *parent = deviceBleSeftPowerRemote->GetParent();
									if (parent)
									{
										if (isAddScene)
										{
											if (bleProtocol->SetSceneSeftPowerRemote(parent->GetAddr(), deviceBleSeftPowerRemote->GetAddr(), convertButton, 1, sceneBle->GetAddr()) != CODE_OK)
												result = CODE_ERROR;
										}
										else
										{
											if (bleProtocol->DelSceneSeftPowerRemote(parent->GetAddr(), deviceBleSeftPowerRemote->GetAddr(), convertButton, 1) != CODE_OK)
												result = CODE_ERROR;
										}
									}
									else
										LOGW("Parent is null");
								}
								else
									LOGW("device seftpower remote is null");
							}
							sizeData--;
						}
					}
				}
			}
			else
			{
				LOGW("SceneBle %s not found", sceneBleId.c_str());
				result = CODE_ERROR;
			}
		}
		else
			result = CODE_ERROR;
		return result;
	}
	else
	{
		LOGW("Data error format");
	}
	return CODE_ERROR;
}

int Gateway::ConfigSceneForPirSensor(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene)
{
	if (!device)
		return CODE_ERROR;
	if (data.isArray() && scene.isObject())
	{
		int result = CODE_OK;
		if (scene.isMember("id") && scene["id"].isString())
		{
			string sceneBleId = scene["id"].asString();
			SceneBle *sceneBle = getSceneBleFromId(sceneBleId);
			if (sceneBle)
			{
				if (isAddScene)
				{
					int pir = 0;
					uint16_t luxHigh = 0;
					uint16_t luxLow = 0;
					bool isPir = false;
					bool isLux = false;
					for (auto &dt : data)
					{
						if (dt.isObject())
						{
							if (dt.isMember("pir") && dt["pir"].isInt())
							{
								pir = dt["pir"].asInt();
								isPir = true;
							}
							if (dt.isMember("lux") && dt["lux"].isArray())
							{
								if (dt["lux"][0].isInt() && dt["lux"][1].isInt())
								{
									luxLow = dt["lux"][0].asInt();
									luxHigh = dt["lux"][1].asInt();
									isLux = true;
								}
							}
							if (isLux && isPir)
								if (bleProtocol->SetScenePirLightSensor(device->GetAddr(), 2, pir, luxLow / 10, luxHigh / 10, sceneBle->GetAddr(), 0) != CODE_OK)
									result = CODE_ERROR;
						}
					}
				}
				else
				{
					for (auto &dt : data)
					{
						if (dt.isMember("pir") && dt["pir"].isInt())
						{
							if (bleProtocol->SetScenePirLightSensor(device->GetAddr(), 2, dt["pir"].asInt(), 0, 0, 0, 0) != CODE_OK)
								result = CODE_ERROR;
						}
					}
				}
			}
		}
		return result;
	}
	else
	{
		LOGW("Data error format");
		return CODE_ERROR;
	}
	return CODE_OK;
}

int Gateway::ConfigSceneForScreenTouch(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene)
{
	if (!device)
		return CODE_ERROR;

	if (scene.isObject() && scene.isMember("id") && scene["id"].isString())
	{
		string sceneBleId = scene["id"].asString();
		SceneBle *sceneBle = getSceneBleFromId(sceneBleId);
		if (sceneBle)
		{
			if (isAddScene)
			{
				if (scene.isMember("icon") && scene["icon"].isInt())
				{
					int iconId = scene["icon"].asInt();
					if (bleProtocol->SceneForScreenTouch(device->GetAddr(), sceneBle->GetAddr(), iconId, 0) == CODE_OK)
						return CODE_OK;
				}
			}
			else
			{
				if (bleProtocol->DelSceneScreenTouch(device->GetAddr(), sceneBle->GetAddr()) == CODE_OK)
					return CODE_OK;
			}
		}
	}
	else
	{
		LOGW("Data error");
	}
	return CODE_ERROR;
}

int Gateway::OnCreateSceneController(Json::Value &reqValue, Json::Value &respValue)
{
	respValue["cmd"] = "createSceneControllerRsp";
	if (reqValue.isMember("devId") && reqValue["devId"].isString() &&
		reqValue.isMember("data") && reqValue["data"].isArray())
	{
		string deviceId = reqValue["devId"].asString();
		Json::Value dataJson = reqValue["data"];
		int result = CODE_OK;

		Device *device = getDeviceFromId(deviceId);
		if (device)
		{
			for (auto &dt : dataJson)
			{
				if (dt.isObject() && dt.isMember("properties") && dt["properties"].isArray() && dt.isMember("scene") && dt["scene"].isObject())
				{
					Json::Value propertiesJson = dt["properties"];
					Json::Value sceneJson = dt["scene"];
					int typeDev = device->GetType() / 1000;
					if (device->GetType() == BLE_REMOTE_M3 || device->GetType() == BLE_REMOTE_M3_V2 || device->GetType() == BLE_REMOTE_M4 || device->GetType() == BLE_DC_SCENE_CONTACT ||
						device->GetType() == BLE_AC_SCENE_CONTACT || device->GetType() == BLE_AC_SCENE_CONTACT_RGB || device->GetType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE ||
						typeDev == 27) // seftpower remote
					{
						result = ConfigSceneForRemote(device, propertiesJson, sceneJson, true);
					}
					else if ((device->GetType() / 1000) == 32)
					{
						result = ConfigSceneForPirSensor(device, propertiesJson, sceneJson, true);
					}
					else if (device->GetType() == BLE_AC_SCENE_SCREEN_TOUCH)
					{
						result = ConfigSceneForScreenTouch(device, propertiesJson, sceneJson, true);
					}
				}
				else
				{
					LOGW("Data error");
				}
			}
			respValue["data"]["code"] = result;
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_DEVICE;
		}
	}
	return CODE_OK;
}

int Gateway::OnDelSceneController(Json::Value &reqValue, Json::Value &respValue)
{
	respValue["cmd"] = "delSceneControllerRsp";
	if (reqValue.isMember("devId") && reqValue["devId"].isString() &&
		reqValue.isMember("data") && reqValue["data"].isArray())
	{
		string deviceId = reqValue["devId"].asString();
		Json::Value dataJson = reqValue["data"];
		int result = CODE_OK;

		Device *device = getDeviceFromId(deviceId);
		if (device)
		{
			for (auto &dt : dataJson)
			{
				if (dt.isObject() && dt.isMember("properties") && dt["properties"].isArray() && dt.isMember("scene") && dt["scene"].isObject())
				{
					Json::Value propertiesJson = dt["properties"];
					Json::Value sceneJson = dt["scene"];
					int typeDev = device->GetType() / 1000;
					if (device->GetType() == BLE_REMOTE_M3 || device->GetType() == BLE_REMOTE_M3_V2 || device->GetType() == BLE_REMOTE_M4 || device->GetType() == BLE_DC_SCENE_CONTACT ||
						device->GetType() == BLE_AC_SCENE_CONTACT || device->GetType() == BLE_AC_SCENE_CONTACT_RGB || device->GetType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE ||
						typeDev == 27) // seftpower remote
					{
						result = ConfigSceneForRemote(device, propertiesJson, sceneJson, false);
					}
					else if ((device->GetType() / 1000) == 32)
					{
						result = ConfigSceneForPirSensor(device, propertiesJson, sceneJson, false);
					}
					else if (device->GetType() == BLE_AC_SCENE_SCREEN_TOUCH)
					{
						result = ConfigSceneForScreenTouch(device, propertiesJson, sceneJson, false);
					}
				}
			}
			respValue["data"]["code"] = result;
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_DEVICE;
		}
	}
	return CODE_OK;
}

int Gateway::OnAddFavoriteScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnAddFavoriteScene");
	if (reqValue.isMember("scenelist") && reqValue["scenelist"].isArray())
	{
		Json::Value scenes = reqValue["scenelist"];
		for (auto &temp : scenes)
		{
			if (temp.isString())
			{
				string temp_scene = temp.asString();
				SceneBle *scene = getSceneBleFromId(temp_scene);
				if (scene)
				{
					// for (const auto &[id, sceneBle] : sceneBleList)
					// {
					// 	if (id == temp_scene)
					// 	{
					scene->SetIsFavorite(true);
					database->SceneBleUpdateFavorite(scene);
					// 	}
					// }
				}
			}
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "addFavoriteSceneRsp";
	return CODE_OK;
}

int Gateway::OnDelFavoriteScene(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("scenelist") && reqValue["scenelist"].isArray())
	{
		Json::Value scenes = reqValue["scenelist"];
		for (auto &temp : scenes)
		{
			if (temp.isString())
			{
				string temp_scene = temp.asString();
				SceneBle *scene = getSceneBleFromId(temp_scene);
				if (scene)
				{
					for (const auto &[id, sceneBle] : sceneBleList)
					{
						if (id == temp_scene)
						{
							sceneBle->SetIsFavorite(false);
							database->SceneBleUpdateFavorite(sceneBle);
						}
					}
				}
			}
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "delFavoriteSceneRsp";
	return CODE_OK;
}

int Gateway::OnGetFavoriteScene(Json::Value &reqValue, Json::Value &respValue)
{
	Json::Value list = Json::arrayValue;
	for (const auto &[id, scene] : sceneBleList)
	{
		if (scene->GetIsFavorite())
		{
			list.append(id);
		}
	}
	respValue["data"]["scenes"] = list;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getFavoriteSceneRsp";
	return CODE_OK;
}

int Gateway::OnUpdateSceneName(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("name") && reqValue["name"].isString() && reqValue.isMember("id") && reqValue["id"].isString())
	{
		string id = reqValue["id"].asString();
		string name = reqValue["name"].asString();
		SceneBle *scene = getSceneBleFromId(id);
		if (scene)
		{
			scene->SetName(name);
			database->SceneBleUpdate(scene);
		}
	}
	Json::Value dataPushToHcApp;
	dataPushToHcApp["cmd"] = "updateSceneName";
	dataPushToHcApp["data"] = reqValue;
	PublishToLocalMessage(dataPushToHcApp);
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "updateSceneNameRsp";
	return CODE_OK;
}
