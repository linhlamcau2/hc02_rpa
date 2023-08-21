#include "Gateway.h"
#include "Log.h"
#include "Db.h"

void Gateway::InitMqttMessageScene()
{
	OnDeviceRpcCallbackRegister("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("createScene", bind(&Gateway::OnCreateScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("delScene", bind(&Gateway::OnDeleteScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("callScene", bind(&Gateway::OnCallScene, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("createScene", bind(&Gateway::OnCreateScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("editScene", bind(&Gateway::OnEditScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delScene", bind(&Gateway::OnDeleteScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("callScene", bind(&Gateway::OnCallScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getSceneList", bind(&Gateway::OnGetSceneList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getDevListInScene", bind(&Gateway::OnGetDevListInScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("addFavoriteScene", bind(&Gateway::OnAddFavoriteScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delFavoriteScene", bind(&Gateway::OnDelFavoriteScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getFavoriteScene", bind(&Gateway::OnGetFavoriteScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("updateSceneName", bind(&Gateway::OnUpdateSceneName, this, placeholders::_1, placeholders::_2));
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
			int rs = sceneBle->Do();
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
	respValue["cmd"] = "controlSceneRsp";
	return CODE_OK;
}

int Gateway::OnGetSceneList(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetSceneList");
	Json::Value sceneData;
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
		Json::Value scenesList;
		Json::Value scenes = reqValue["scenes"];
		for (auto &sceneValue : scenes)
		{
			Json::Value scenesData;
			if (sceneValue.isString())
			{
				string sceneId = sceneValue.asString();
				scenesData["id"] = sceneId;
				SceneBle *temp_scene = getSceneBleFromId(sceneId);
				if (temp_scene)
				{
					Json::Value temp_devicesList;
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
								// database->DeviceInSceneBleAdd(sceneBle, device, deviceProperties.toString());
								successList.append(device->GetId());
							}
							else
							{
								failedList.append(device->GetId());
							}
						}
					}
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
				respValue["data"]["code"] = CODE_OK;
				respValue["data"]["addr"] = sceneAddr;
				respValue["data"]["success"] = successList;
				respValue["data"]["failed"] = failedList;

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
		if (sceneBle)
		{
			vector<Device *> devsInScene; // list dev in scene
			for (auto &devs : sceneBle->deviceList)
			{
				devsInScene.push_back(devs->device);
			}
			vector<Device *> listDevicesAdd; // list new dev add scene
			vector<Device *> listDevicesDel; // list old dev del scene
			vector<Device *> devsEditScene;	 // list dev in msg edit scene

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
			
			//Find list deivce add scene
			for (auto &item : devsEditScene)
			{
				if (find(devsInScene.begin(), devsInScene.end(), item) == devsInScene.end())
				{
					listDevicesAdd.push_back(item);
				}
			}

			for (auto &item : listDevicesAdd)
			{
				if (sceneBle->AddDevice(item, listData[item], true, true) == CODE_OK)
				{
					successList.append(item->GetId());
				}
				else
				{
					failedList.append(item->GetId());
				}
			}

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
			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["id"] = sceneId;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;

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
		Json::Value successList;
		Json::Value failedList;
		string sceneId = reqValue["id"].asString();
		SceneBle *sceneBle = getSceneBleFromId(sceneId);
		if (sceneBle)
		{
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
			}
			delSceneBle(sceneBle);

			printScene();

			respValue["data"]["code"] = CODE_OK;
			respValue["data"]["success"] = successList;
			respValue["data"]["failed"] = failedList;
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

int Gateway::OnAddFavoriteScene(Json::Value &reqValue, Json::Value &respValue)
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
	Json::Value list;
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
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "updateSceneNameRsp";
	return CODE_OK;
}
