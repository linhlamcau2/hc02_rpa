#if CONFIG_USE_MESSAGE_FORMAT_V2
#include "Gateway.h"
#include "Log.h"
#include "Db.h"
#include "Util.h"
#include "Ota.h"
#include "BleProtocol.h"
#include "BleDefine.h"
#include "Http.h"
#include "Base64.h"
#include "Wifi.h"
#include <fstream>

#define ROOM_START_ADDR 0xD000

void Gateway::initMqttMessageV2()
{
	OnDeviceRpcCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlGw", bind(&Gateway::OnControlGw, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlGroup", bind(&Gateway::OnControlGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("getDevStt", bind(&Gateway::OnGetDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("getAllDevStt", bind(&Gateway::OnGetAllDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("getDevList", bind(&Gateway::OnGetDeviceList, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegisterV2("getHcInfo", bind(&Gateway::OnGetHcInfo, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("startScanBle", bind(&Gateway::OnStartScanBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("stopScanBle", bind(&Gateway::OnStopScanBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("createGroup", bind(&Gateway::OnCreateGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("addDevToGroup", bind(&Gateway::OnAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("delDevFromGroup", bind(&Gateway::OnDeleteDeviceFromGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("delGroup", bind(&Gateway::OnDeleteGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("createScene", bind(&Gateway::OnCreateScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("delScene", bind(&Gateway::OnDeleteScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("callScene", bind(&Gateway::OnCallScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("createRule", bind(&Gateway::OnCreateRule, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegisterV2("delRule", bind(&Gateway::OnDeleteRule, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegisterV2("addDevToRoom", bind(&Gateway::OnAddDeviceToRoom, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegisterV2("delDevFromRoom", bind(&Gateway::OnDeleteDeviceFromRoom, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegisterV2("delRoom", bind(&Gateway::OnDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("SSHRemote", bind(&Gateway::OnSSHRemote, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlGw", bind(&Gateway::OnControlGw, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlGroup", bind(&Gateway::OnControlGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getDevStt", bind(&Gateway::OnGetDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getAllDevStt", bind(&Gateway::OnGetAllDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getDevList", bind(&Gateway::OnGetDeviceList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getHcInfo", bind(&Gateway::OnGetHcInfo, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("startScanBle", bind(&Gateway::OnStartScanBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("stopScanBle", bind(&Gateway::OnStopScanBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("createGroup", bind(&Gateway::OnCreateGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("addDevToGroup", bind(&Gateway::OnAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delDevFromGroup", bind(&Gateway::OnDeleteDeviceFromGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delGroup", bind(&Gateway::OnDeleteGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("createScene", bind(&Gateway::OnCreateScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delScene", bind(&Gateway::OnDeleteScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("callScene", bind(&Gateway::OnCallScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("createRule", bind(&Gateway::OnCreateRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delRule", bind(&Gateway::OnDeleteRule, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegisterV2("addDevToRoom", bind(&Gateway::OnAddDeviceToRoom, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegisterV2("delDevFromRoom", bind(&Gateway::OnDeleteDeviceFromRoom, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegisterV2("delRoom", bind(&Gateway::OnDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getRoomList", bind(&Gateway::OnGetRoomList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getDevListInRoom", bind(&Gateway::OnGetDevListInRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getGroupList", bind(&Gateway::OnGetGroupList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getSceneList", bind(&Gateway::OnGetSceneList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getRuleList", bind(&Gateway::OnGetRuleList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getRuleInfo", bind(&Gateway::OnGetRuleInfo, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getDevListInScene", bind(&Gateway::OnGetDevListInScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("createRoom", bind(&Gateway::OnCreateRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("addDevToRoom", bind(&Gateway::OnAddDeviceToRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delDevToRoom", bind(&Gateway::OnDeleteDeviceFromRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delRoom", bind(&Gateway::OnDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("SSHRemote", bind(&Gateway::OnSSHRemote, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("actionRule", bind(&Gateway::OnActionRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getGroupIntoRoom", bind(&Gateway::OnGetGroupIntoRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("getSceneIntoRoom", bind(&Gateway::OnGetSceneIntoRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delDev", bind(&Gateway::OnDeleteDevice, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnControlDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlDevice");
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
			reqValue.isMember("data") && reqValue["data"].isObject())
	{
		string deviceId = reqValue["id"].asString();
		Json::Value devData = reqValue["data"];
		Device *device = getDeviceFromId(deviceId);
		if (device)
		{
			int rs = device->DoV2(devData);
			respValue["data"]["code"] = rs;
		}
		else
		{
			LOGW("Device id %s not found", deviceId.c_str());
			respValue["data"]["code"] = CODE_NOT_FOUND_DEVICE;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlDevice %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "controlDevRsp";
	return CODE_OK;
}

int Gateway::OnControlAllDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlAllDevice");
	if (bleProtocol)
	{
		if (reqValue.isMember(KEY_ATTRIBUTE_ONOFF) && reqValue[KEY_ATTRIBUTE_ONOFF].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_ONOFF].asInt();
			bleProtocol->SetOnOffLight(0xFFFF, value, 0, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_DIM) && reqValue[KEY_ATTRIBUTE_DIM].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_DIM].asInt();
			uint16_t dim = (value * 65535) / 100;
			bleProtocol->SetDimmingLight(0xFFFF, dim, 0, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_CCT) && reqValue[KEY_ATTRIBUTE_CCT].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_CCT].asInt();
			uint16_t cct = (value * 192) + 800;
			bleProtocol->SetCctLight(0xFFFF, cct, 0, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_HUE) && reqValue[KEY_ATTRIBUTE_HUE].isInt() &&
				reqValue.isMember(KEY_ATTRIBUTE_SATURATION) && reqValue[KEY_ATTRIBUTE_SATURATION].isInt() &&
				reqValue.isMember(KEY_ATTRIBUTE_LUMINANCE) && reqValue[KEY_ATTRIBUTE_LUMINANCE].isInt())
		{
			int h = reqValue[KEY_ATTRIBUTE_HUE].asInt();
			int s = reqValue[KEY_ATTRIBUTE_SATURATION].asInt();
			int l = reqValue[KEY_ATTRIBUTE_LUMINANCE].asInt();
			bleProtocol->SetHSLLight(0xFFFF, h, s, l, 0, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && reqValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
			bleProtocol->CallModeRgb(0xFFFF, value);
		}
		respValue["data"]["code"] = CODE_OK;
	}
	respValue["cmd"] = "controlAllDevRsp";
	return CODE_OK;
}

int Gateway::OnControlGw(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlGw");
	int rs = Do(reqValue);
	respValue["data"]["code"] = rs;
	respValue["cmd"] = "controlGwRsp";
	return CODE_OK;
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
			int rs = group->DoV2(devData);
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

int Gateway::OnControlScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlScene");
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string sceneId = reqValue["id"].asString();
		SceneBle *scene = getSceneBleFromId(sceneId);
		if (scene)
		{
			int rs = scene->Do();
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

int Gateway::OnGetDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetDeviceStatus");
	if (reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value devicesData;
		Json::Value devices = reqValue["devices"];
		for (auto &deviceValue : devices)
		{
			if (deviceValue.isString())
			{
				string deviceId = deviceValue.asString();
				Device *device = getDeviceFromId(deviceId);
				if (device)
				{
					Json::Value deviceValue;
					deviceValue["id"] = device->GetId();
					Json::Value deviceAttbute;
					device->BuildTelemetryValueV2(deviceAttbute);
					deviceValue["data"] = deviceAttbute;
					devicesData.append(deviceValue);
				}
			}
		}
		respValue["data"]["code"] = CODE_OK;
		respValue["data"]["devices"] = devicesData;
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "deviceUpdate";
	return CODE_OK;
}

int Gateway::OnGetAllDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetAllDeviceStatus");
	Json::Value devicesData;
	AddAllDeviceStatusV2(devicesData);
	respValue["data"]["code"] = CODE_OK;
	respValue["data"]["devices"] = devicesData;
	respValue["cmd"] = "deviceUpdate";
	return CODE_OK;
}

int Gateway::OnGetDeviceList(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetDeviceList");
	Json::Value devicesData;
	for (const auto &[id, device] : deviceList)
	{
		Json::Value deviceValue;
		deviceValue["id"] = device->GetId();
		deviceValue["addr"] = (Json::UInt)device->GetAddr();
		deviceValue["type"] = (Json::UInt)device->GetType();
		deviceValue["mac"] = device->GetMac();
		deviceValue["ver"] = device->GetVersionStr();
		devicesData.append(deviceValue);
	}
	respValue["data"]["devices"] = devicesData;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getDevListRsp";
	return CODE_OK;
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
						DeviceInRoom *deviceInRoom = temp_room->deviceList[i];
						string deviceId = deviceInRoom->device->GetId();
						temp_devicesList.append(deviceId);
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

int Gateway::OnGetRuleList(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetRuleList");
	Json::Value ruleData;
	ruleListMtx.lock();
	for (const auto &[id, rule] : ruleList)
	{
		Json::Value ruleValue;
		ruleValue["id"] = rule->GetId();
		ruleValue["name"] = rule->GetName();
		ruleData.append(ruleValue);
	}
	ruleListMtx.unlock();
	respValue["data"]["rules"] = ruleData;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getRuleListRsp";
	return CODE_OK;
}
int Gateway::OnGetRuleInfo(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnOnGetRuleInfo");
	if (reqValue.isMember("rules") && reqValue["rules"].isArray() && reqValue["rules"].size() > 0)
	{
		Json::Value rules = reqValue["rules"];
		Json::Value ruleData;
		for (auto &ruleValue : rules)
		{
			if (ruleValue.isString())
			{
				string ruleId = ruleValue.asString();
				Rule *temp_rule = getRuleFromId(ruleId);
				if (temp_rule)
				{
					cout << temp_rule->GetRuleData() << endl;
					ruleData.append(temp_rule->GetRuleData());
					respValue["data"]["code"] = CODE_OK;
				}
			}
			else
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
		}
		respValue["data"]["rules"] = ruleData;
	}
	respValue["cmd"] = "getRuleInfoRsp";
	return CODE_OK;
}

int Gateway::OnGetHcInfo(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetHcInfo");
	Json::Value dataValue;
	dataValue["mac"] = mac;
	dataValue["ip"] = Wifi::GetIP();
	dataValue["name"] = "RD HC";
	dataValue["type"] = MODEL;
	dataValue["ver"] = STR(VERSION);
	respValue["data"] = dataValue;
	respValue["cmd"] = "getHcInfoRsp";
	return CODE_OK;
}

int Gateway::OnStartScanBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (bleProtocol)
	{
		bleProtocol->StartScan();
		respValue["data"]["code"] = CODE_OK;
		respValue["cmd"] = "startScanBleRsp";
		return CODE_OK;
	}
	else
		LOGW("BleProtocol null");
	return CODE_ERROR;
}

int Gateway::OnStopScanBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (bleProtocol)
	{
		bleProtocol->StopScan();
		respValue["data"]["code"] = CODE_OK;
		respValue["cmd"] = "stopScanBleRsp";
		return CODE_OK;
	}
	else
		LOGW("BleProtocol null");
	return CODE_ERROR;
}

int Gateway::OnDeleteDevice(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("device") && reqValue["device"].isArray())
	{
		Json::Value successList;
		Json::Value failedList;
		Json::Value devices = reqValue["device"];
		for (auto &deviceValue : devices)
		{
			if (deviceValue.isString())
			{
				string deviceId = deviceValue.asString();
				Device *device = getDeviceFromId(deviceId);
				if (device)
				{
					if (bleProtocol)
					{
						bleProtocol->ResetDev(device->GetAddr());
						database->DeviceDel(device->GetMac());
						LOGD("remove deviceId: %s", deviceId.c_str());
						successList.append(deviceId);
					}
					else
						LOGW("BleProtocol null");
				}
				else
				{
					LOGD("deviceId %s dose not exist", deviceId.c_str());
					failedList.append(deviceId);
				}
			}
		}
		respValue["data"]["code"] = 0;
		respValue["data"]["success"] = successList;
		respValue["data"]["failed"] = failedList;
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "delDevRsp";
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
				if (group->AddDevice(device, deviceAddr, true))
				{
					database->DeviceInGroupAdd(group, device, deviceAddr);
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
		Json::Value successList;
		Json::Value failedList;
		string groupId = reqValue["id"].asString();
		string groupName = reqValue["name"].asString();
		Json::Value devices = reqValue["devices"];
		// TODO: add start address of normal group
		int groupAddr = 1;
		groupListMtx.lock();
		for (const auto &[id, group] : groupList)
		{
			if (group->GetAddr() >= groupAddr)
			{
				groupAddr = group->GetAddr() + 1;
			}
		}
		groupListMtx.unlock();
		Group *group = new Group(groupId, groupAddr, groupName);
		if (group)
		{
			if (reqValue.isMember("roomId") && reqValue["roomId"].isString())
			{
				string roomId = reqValue["roomId"].asString();
				Room *room = getRoomFromId(roomId);
				if (room)
				{
					room->AddGroup(group, true, true);
				}
			}
			if (AddNewGroup(group, true, true))
			{
				for (auto &deviceValue : devices)
				{
					if (deviceValue.isString())
					{
						string deviceId = deviceValue.asString();
						Device *device = getDeviceFromId(deviceId);
						if (device)
						{
							int deviceAddr = device->GetAddr();
							if (group->AddDevice(device, deviceAddr, true) == CODE_OK)
							{
								database->DeviceInGroupAdd(group, device, deviceAddr);
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
				respValue["data"]["id"] = groupId;
				respValue["data"]["success"] = successList;
				respValue["data"]["failed"] = failedList;
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
		Json::Value successList;
		Json::Value failedList;
		string groupId = reqValue["id"].asString();
		Json::Value devices = reqValue["devices"];
		Group *group = getGroupFromId(groupId);
		if (group)
		{
			for (auto &deviceValue : devices)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						int deviceAddr = device->GetAddr();
						if (group->AddDevice(device, deviceAddr, true) == CODE_OK)
						{
							database->DeviceInGroupAdd(group, device, deviceAddr);
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
				if (group->DelDevice(device, deviceAddr) == CODE_OK)
				{
					database->DeviceInGroupDel(group, device, deviceAddr);
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
		Json::Value successList;
		Json::Value failedList;
		string groupId = reqValue["id"].asString();
		Json::Value devices = reqValue["devices"];
		Group *group = getGroupFromId(groupId);
		if (group)
		{
			for (auto &deviceValue : devices)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						int deviceAddr = device->GetAddr();
						if (group->DelDevice(device, deviceAddr))
						{
							database->DeviceInGroupDel(group, device, deviceAddr);
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
			for (auto &deviceInGroup : group->deviceList)
			{
				if (group->DelDevice(deviceInGroup->device, deviceInGroup->device->GetAddr()))
				{
					database->DeviceInGroupDel(group, deviceInGroup->device, deviceInGroup->device->GetAddr());
					successList.append(deviceInGroup->device->GetId());
				}
				else
				{
					failedList.append(deviceInGroup->device->GetId());
				}
			}
			delGroup(group);
			
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

int Gateway::OnCreateScene(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
			reqValue.isMember("name") && reqValue["name"].isString() &&
			reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value successList;
		Json::Value failedList;
		string sceneId = reqValue["id"].asString();
		string sceneName = reqValue["name"].asString();
		// TODO: add start address of normal scene
		int sceneAddr = 1;
		sceneBleListMtx.lock();
		for (const auto &[id, sceneBle] : sceneBleList)
		{
			if (sceneBle->GetAddr() >= sceneAddr)
			{
				sceneAddr = sceneBle->GetAddr() + 1;
			}
		}
		sceneBleListMtx.unlock();
		SceneBle *scene = new SceneBle(sceneId, sceneAddr, sceneName);
		if (scene)
		{
			if (reqValue.isMember("roomId") && reqValue["roomId"].isString())
			{
				string roomId = reqValue["roomId"].asString();
				Room *room = getRoomFromId(roomId);
				if (room)
				{
					room->AddSceneBle(scene, true, true);
				}
			}
			if (AddNewSceneBle(scene, true, true))
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
							if (scene->AddDeviceV2(device, deviceProperties, false) == CODE_OK)
							{
								database->DeviceInSceneBleAdd(scene, device, deviceProperties.toString());
								successList.append(device->GetId());
							}
							else
							{
								failedList.append(device->GetId());
							}
						}
					}
				}
				respValue["data"]["code"] = CODE_OK;
				respValue["data"]["id"] = sceneAddr;
				respValue["data"]["success"] = successList;
				respValue["data"]["failed"] = failedList;
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

int Gateway::OnDeleteScene(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		Json::Value successList;
		Json::Value failedList;
		string sceneId = reqValue["id"].asString();
		SceneBle *scene = getSceneBleFromId(sceneId);
		if (scene)
		{
			for (auto &deviceInScene : scene->deviceList)
			{
				if (scene->DelDevice(deviceInScene->device) == CODE_OK)
				{
					database->DeviceInSceneBleDel(scene, deviceInScene->device);
					successList.append(deviceInScene->device->GetId());
				}
				else
				{
					failedList.append(deviceInScene->device->GetId());
				}
			}
			delSceneBle(scene);

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
		SceneBle *scene = getSceneBleFromId(sceneId);
		if (scene)
		{
			scene->Do();
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

int Gateway::OnCreateRule(Json::Value &reqValue, Json::Value &respValue)
{
	Rule *rule = AddRuleV2(reqValue);
	if (rule)
	{
		LOGI("Add Rule %s", rule->GetId().c_str());
		ruleListMtx.lock();
		ruleList[rule->GetId()] = rule;
		ruleListMtx.unlock();
		string ruleStr = reqValue.toString();
		ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
		database->RuleAdd(rule, ruleStr, true);
		rule->Check();
		respValue["data"]["code"] = CODE_OK;
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "createRuleRsp";
	return CODE_OK;
}

int Gateway::OnDeleteRule(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string ruleId = reqValue["id"].asString();
		Rule *rule = getRuleFromId(ruleId);
		if (rule)
		{
			delRule(rule);
			respValue["data"]["code"] = CODE_OK;
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_RULE;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "delRuleRsp";
	return CODE_OK;
}

int Gateway::OnCreateRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
			reqValue.isMember("name") && reqValue["name"].isString() &&
			reqValue.isMember("devices") && reqValue["devices"].isArray() &&
			reqValue.isMember("scenes") && reqValue["scenes"].isArray())
	{
		Json::Value successList;
		Json::Value failedList;
		string roomId = reqValue["id"].asString();
		string roomName = reqValue["name"].asString();
		Json::Value devices = reqValue["devices"];
		Json::Value scenes = reqValue["scenes"];
		int roomAddr = ROOM_START_ADDR;

		roomListMtx.lock();
		for (const auto &[id, room] : roomList)
		{
			if (room->GetAddr() >= roomAddr)
			{
				roomAddr = room->GetAddr() + 200;
			}
		}
		roomListMtx.unlock();
		LOGD("roomAddr: %d", roomAddr);
		Room *room = new Room(roomId, roomAddr, roomName);
		Group *group = new Group(roomId, roomAddr - ROOM_START_ADDR, roomName);
		if (room)
		{
			if (AddNewRoom(room, true, true))
				if (group)
					if (AddNewGroup(group, true, true))
						room->AddGroup(group, true, true);

			for (auto &deviceValue : devices)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						int epId = device->GetAddr();
						database->DeviceInRoomAdd(room, device);
						database->DeviceInGroupAdd(group, device, epId);
						room->AddDevice(device, false);
						group->AddDevice(device, epId, true);
						successList.append(device->GetId());
					}
					else
					{
						failedList.append(deviceId);
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
		Json::Value devices = reqValue["devices"];
		Room *room = getRoomFromId(roomId);
		if (room)
		{
			for (auto &deviceValue : devices)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						if (room->AddDevice2(device, true))
						{
							database->DeviceInRoomAdd(room, device);
							successList.append(deviceId);
						}
						else
						{
							LOGD("add to room deviceId %s error", deviceId.c_str());
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
		Json::Value devices = reqValue["devices"];
		Room *room = getRoomFromId(roomId);
		if (room)
		{
			for (auto &deviceValue : devices)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						if (room->DelDevice2(device))
						{
							database->DeviceInRoomDel(room, device);
							successList.append(deviceId);
						}
						else
						{
							LOGD("delete from room deviceId %s error", deviceId.c_str());
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
				if (room->DelDevice2(deviceInRoom->device))
				{
					database->DeviceInRoomDel(room, deviceInRoom->device);
					successList.append(deviceInRoom->device->GetId());
				}
				else
				{
					LOGD("delete from room deviceId %s error", deviceInRoom->device->GetId().c_str());
					failedList.append(deviceInRoom->device->GetId());
				}
			}
			database->RoomDel(room);
			delete room;
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

int Gateway::OnResetHC(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("OnResetFactory");
	ResetFactory();
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "resetHcRsp";
	return CODE_OK;
}

int Gateway::OnSSHRemote(Json::Value &reqValue, Json::Value &respValue)
{
	int err = 0;
	if (reqValue.isMember("type") && reqValue["type"].isString() &&
			reqValue.isMember("key") && reqValue["key"].isString() &&
			reqValue.isMember("user") && reqValue["user"].isString() &&
			reqValue.isMember("host") && reqValue["host"].isString() &&
			reqValue.isMember("serverPort") && reqValue["serverPort"].isInt() &&
			reqValue.isMember("forwardPort") && reqValue["forwardPort"].isInt())
	{
		string key = "";
		string type = reqValue["type"].asString();
		string user = reqValue["user"].asString();
		string host = reqValue["host"].asString();
		uint32_t serverPort = reqValue["serverPort"].asInt();
		uint32_t forwardPort = reqValue["forwardPort"].asInt();
		uint32_t localPort = 22;
		if (reqValue.isMember("localPort") && reqValue["localPort"].isInt())
		{
			localPort = reqValue["localPort"].asInt();
		}
		if (type == "base64")
		{
			string keyBase64 = reqValue["key"].asString();
			string decode = macaron::Base64::Decode(keyBase64, key);
			if (decode != "")
			{
				err = 1;
				LOGW("Base64 decode err: %s", decode.c_str());
			}
		}
		else
		{
			key = reqValue["key"].asString();
		}

		if (err == 0)
		{
			// save key file
			system("rm /key.txt");
			system("rm /output.txt");
			ofstream keyFile("/key.txt");
			keyFile << key;
			keyFile.close();

			system("chmod 600 /key.txt");
			system("killall ssh");
			string cmd = "ssh -i /key.txt -o StrictHostKeyChecking=no -f -N -T -R" + to_string(forwardPort) + ":localhost:" + to_string(localPort) + " " + user + "@" + host + " -p " + to_string(serverPort);
			cmd += " >> /output.txt 2>&1";
			LOGI("cmd: %s", cmd.c_str());
			system(cmd.c_str());
			sleep(2);
			bool err = false;
			FILE *fp = fopen("/output.txt", "r");
			char path[512] = {0};
			if (fp)
			{
				while (fgets(path, sizeof(path), fp) != NULL)
				{
					if (strlen(path) > 1)
					{
						LOGW("SSH err: %s", path);
						err = true;
						break;
					}
				}
				fclose(fp);
			}
			if (err)
			{
				respValue["msg"] = string(path);
				respValue["code"] = 1;
			}
			else
			{
				respValue["code"] = 0;
			}
			return CODE_OK;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["code"] = err;
	return CODE_OK;
}

int Gateway::OnActionRule(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string id = reqValue["id"].asString();
		Rule *rule = getRuleFromId(id);
		if (rule)
		{
			rule->RunOutput();
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "actionRuleRsp";
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

#endif // CONFIG_USE_MESSAGE_FORMAT_V2
