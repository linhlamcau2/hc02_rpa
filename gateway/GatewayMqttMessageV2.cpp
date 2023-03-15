#include "Gateway.h"
#include "Log.h"
#include "Db.h"
#include "Util.h"
#include "Ota.h"
#include "BleProtocol.h"
#include "BleDefine.h"
#include "Http.h"
#include "Base64.h"
#include <fstream>

#define ROOM_START_ADDR 0xD000

void Gateway::initMqttMessageV2()
{
	OnDeviceRpcCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlGroup", bind(&Gateway::OnControlGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("requestDev", bind(&Gateway::OnRequestDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("startScanBle", bind(&Gateway::OnStartScanBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("stopScanBle", bind(&Gateway::OnStopScanBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("createGroup", bind(&Gateway::OnCreateGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("addDevToGroup", bind(&Gateway::OnAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("delDevFromGroup", bind(&Gateway::OnDeleteDeviceFromGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("delGroup", bind(&Gateway::OnDeleteGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("createScene", bind(&Gateway::OnCreateScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("delScene", bind(&Gateway::OnDeleteScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("callScene", bind(&Gateway::OnCallScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("createRule", bind(&Gateway::OnCreateRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("delRule", bind(&Gateway::OnDeleteRule, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlGroup", bind(&Gateway::OnControlGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("requestDev", bind(&Gateway::OnRequestDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("startScanBle", bind(&Gateway::OnStartScanBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("stopScanBle", bind(&Gateway::OnStopScanBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("createGroup", bind(&Gateway::OnCreateGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("addDevToGroup", bind(&Gateway::OnAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delDevFromGroup", bind(&Gateway::OnDeleteDeviceFromGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delGroup", bind(&Gateway::OnDeleteGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("createScene", bind(&Gateway::OnCreateScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delScene", bind(&Gateway::OnDeleteScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("callScene", bind(&Gateway::OnCallScene, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("createRule", bind(&Gateway::OnCreateRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delRule", bind(&Gateway::OnDeleteRule, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnControlDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlDevice");
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("data") && data["data"].isObject())
		{
			string deviceId = data["id"].asString();
			Json::Value devData = data["data"];
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
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("data") && data["data"].isObject())
		{
			Json::Value dataValue = data["data"];
			if (dataValue.isObject())
			{
				if (dataValue.isMember(KEY_ATTRIBUTE_ONOFF) && dataValue[KEY_ATTRIBUTE_ONOFF].isInt())
				{
					int value = dataValue[KEY_ATTRIBUTE_ONOFF].asInt();
					bleProtocol->SetOnOffLight(0xFFFF, value, 0, true);
				}
				if (dataValue.isMember(KEY_ATTRIBUTE_DIM) && dataValue[KEY_ATTRIBUTE_DIM].isInt())
				{
					int value = dataValue[KEY_ATTRIBUTE_DIM].asInt();
					uint16_t dim = (value * 65535) / 100;
					bleProtocol->SetDimmingLight(0xFFFF, dim, 0, true);
				}
				if (dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
				{
					int value = dataValue[KEY_ATTRIBUTE_CCT].asInt();
					uint16_t cct = (value * 192) + 800;
					bleProtocol->SetCctLight(0xFFFF, cct, 0, true);
				}
				if (dataValue.isMember(KEY_ATTRIBUTE_HUE) && dataValue[KEY_ATTRIBUTE_HUE].isInt() &&
						dataValue.isMember(KEY_ATTRIBUTE_SATURATION) && dataValue[KEY_ATTRIBUTE_SATURATION].isInt() &&
						dataValue.isMember(KEY_ATTRIBUTE_LUMINANCE) && dataValue[KEY_ATTRIBUTE_LUMINANCE].isInt())
				{
					int h = dataValue[KEY_ATTRIBUTE_HUE].asInt();
					int s = dataValue[KEY_ATTRIBUTE_SATURATION].asInt();
					int l = dataValue[KEY_ATTRIBUTE_LUMINANCE].asInt();
					bleProtocol->SetHSLLight(0xFFFF, h, s, l, 0, true);
				}
				if (dataValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && dataValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
				{
					int value = dataValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
					bleProtocol->CallModeRgb(0xFFFF, value);
				}
				respValue["data"]["code"] = 0;
			}
			else
			{
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
			}
		}
		else
		{
			respValue["data"]["code"] = CODE_FORMAT_ERROR;
			LOGW("OnControlDevice %s format error", reqValue.toString().c_str());
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlDevice %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "controlAllDevRsp";
	return CODE_OK;
}

int Gateway::OnControlGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlGroup");
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("data") && data["data"].isObject())
		{
			string groupId = data["id"].asString();
			Json::Value devData = data["data"];
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
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString())
		{
			string sceneId = data["id"].asString();
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
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlScene %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "controlSceneRsp";
	return CODE_OK;
}

int Gateway::OnRequestDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRequestDeviceStatus");
	Json::Value deviceData;
	AddAllDeviceStatusV2(deviceData);
	respValue["data"]["code"] = CODE_OK;
	respValue["data"]["device"] = deviceData;
	respValue["cmd"] = "requestDevSttRsp";
	return CODE_OK;
}

int Gateway::OnStartScanBle(Json::Value &reqValue, Json::Value &respValue)
{
	bleProtocol->isAdding = true;
	bleProtocol->isProvisioning = true;
	if (bleProtocol->StartScan())
	{
		bleProtocol->StopScan();
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "startScanBleRsp";
	return CODE_OK;
}

int Gateway::OnStopScanBle(Json::Value &reqValue, Json::Value &respValue)
{
	bleProtocol->StopScan();
	bleProtocol->isAdding = false;
	bleProtocol->isProvisioning = false;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "stopScanBleRsp";
	return CODE_OK;
}

int Gateway::OnDeleteDevice(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("devices") && data["devices"].isArray())
		{
			Json::Value successList;
			Json::Value failedList;
			Json::Value devices = reqValue["devices"];
			for (auto &deviceValue : devices)
			{
				if (deviceValue.isString())
				{
					string deviceId = deviceValue.asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						if (bleProtocol->ResetDev(device->GetAddr()) == CODE_OK)
						{
							database->DeviceDel(device->GetMac());
							LOGD("remove deviceId: %s", deviceId.c_str());
							successList.append(deviceId);
						}
						else
						{
							LOGD("delete deviceId %s error", deviceId.c_str());
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
			respValue["data"]["code"] = CODE_FORMAT_ERROR;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "delDevRsp";
	return CODE_OK;
}

int Gateway::OnCreateGroup(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("name") && data["name"].isString() &&
				data.isMember("devices") && data["devices"].isArray())
		{
			Json::Value successList;
			Json::Value failedList;
			string groupId = data["id"].asString();
			string groupName = data["name"].asString();
			Json::Value devices = reqValue["devices"];
			// TODO: add start address of normal group
			int groupAddr = 1;
			for (const auto &[id, group] : groupList)
			{
				if (group->GetAddr() >= groupAddr)
				{
					groupAddr = group->GetAddr() + 1;
				}
			}
			Group *group = new Group(groupId, groupAddr, groupName);
			if (group)
			{
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
								if (group->AddDevice(device, deviceAddr, true))
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
					respValue["data"]["addr"] = groupAddr;
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
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("devices") && data["devices"].isArray())
		{
			Json::Value successList;
			Json::Value failedList;
			string groupId = data["id"].asString();
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
							if (group->AddDevice(device, deviceAddr, true))
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
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "addDevToGroupRsp";
	return CODE_OK;
}

int Gateway::OnDeleteDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("devices") && data["devices"].isArray())
		{
			Json::Value successList;
			Json::Value failedList;
			string groupId = data["id"].asString();
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
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString())
		{
			Json::Value successList;
			Json::Value failedList;
			string groupId = data["id"].asString();
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
				database->GroupDel(group);
				groupList.erase(group->GetId());
				delete group;
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
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("name") && data["name"].isString() &&
				data.isMember("devices") && data["devices"].isArray())
		{
			Json::Value successList;
			Json::Value failedList;
			string sceneId = data["id"].asString();
			string sceneName = data["name"].asString();
			// TODO: add start address of normal scene
			int sceneAddr = 1;
			for (const auto &[id, sceneBle] : sceneBleList)
			{
				if (sceneBle->GetAddr() >= sceneAddr)
				{
					sceneAddr = sceneBle->GetAddr() + 1;
				}
			}
			SceneBle *scene = new SceneBle(sceneId, sceneAddr, sceneName);
			if (scene)
			{
				if (AddNewSceneBle(scene, true, true))
				{
					Json::Value groupList = data["devices"];
					for (auto &groupValue : groupList)
					{
						if (groupValue.isObject() &&
								groupValue.isMember("id") && groupValue["id"].isArray() &&
								groupValue.isMember("data") && groupValue["data"].isObject())
						{
							Json::Value deviceList = groupValue["id"];
							Json::Value deviceProperties = groupValue["data"];
							for (auto &deviceValue : deviceList)
							{
								if (deviceValue.isString())
								{
									string devcieId = deviceValue.asString();
									Device *device = getDeviceFromId(devcieId);
									if (device)
									{
										if (scene->AddDeviceV2(device, deviceProperties, false))
										{
											database->DeviceInSceneBleAdd(scene, device, deviceProperties);
											successList.append(device->GetId());
										}
										else
										{
											failedList.append(device->GetId());
										}
									}
								}
							}
						}
					}
					respValue["data"]["code"] = CODE_OK;
					respValue["data"]["addr"] = sceneAddr;
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
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString())
		{
			Json::Value successList;
			Json::Value failedList;
			string sceneId = data["id"].asString();
			SceneBle *scene = getSceneBleFromId(sceneId);
			if (scene)
			{
				for (auto &deviceInScene : scene->deviceList)
				{
					if (scene->DelDevice(deviceInScene->device))
					{
						database->DeviceInSceneBleDel(scene, deviceInScene->device, deviceInScene->device->GetAddr());
						successList.append(deviceInScene->device->GetId());
					}
					else
					{
						failedList.append(deviceInScene->device->GetId());
					}
				}
				sceneBleList.erase(scene->GetId());
				database->SceneBleDel(scene);
				delete scene;
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
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString())
		{
			string sceneId = data["id"].asString();
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
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		Rule *rule = AddRuleV2(data);
		if (rule)
		{
			LOGI("Add Rule %s", rule->GetId().c_str());
			ruleList[rule->GetId()] = rule;
			string ruleStr = data.toString();
			ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
			database->RuleAdd(rule->GetId(), ruleStr, true, 1);
			rule->Check();
			respValue["data"]["code"] = CODE_OK;
		}
		else
		{
			respValue["data"]["code"] = CODE_FORMAT_ERROR;
		}
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
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString())
		{
			string ruleId = data["id"].asString();
			Rule *rule = getRuleFromId(ruleId);
			if (rule)
			{
				database->RuleDel(ruleId);
				delete ruleList[ruleId];
				ruleList.erase(ruleId);
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
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("name") && data["name"].isString() &&
				data.isMember("devices") && data["devices"].isArray() &&
				data.isMember("scenes") && data["scenes"].isArray())
		{
			Json::Value successList;
			Json::Value failedList;
			string roomId = data["id"].asString();
			string roomName = data["name"].asString();
			Json::Value devices = reqValue["devices"];
			Json::Value scenes = reqValue["scenes"];
			int roomAddr = ROOM_START_ADDR;
			for (const auto &[id, room] : roomList)
			{
				if (room->GetAddr() >= roomAddr)
				{
					roomAddr = room->GetAddr() + 200;
				}
			}
			LOGD("roomAddr: %d", roomAddr);
			Room *room = new Room(roomId, roomAddr, roomName);
			if (room)
			{
				if (AddNewRoom(room))
				{
					// string sceneNames[] = {"Cảnh 1", "Cảnh 2", "Cảnh 3", "Cảnh 4", "Cảnh 5", "Cảnh 6"};
					for (int i = 0; i < scenes.size(); i++)
					{
						if (scenes[i].isString())
						{
							string sceneId = scenes[i].asString();
							SceneBle *scene = new SceneBle(sceneId, roomAddr + i + 1, "Cảnh " + to_string(i + 1));
							if (scene)
							{
								AddNewSceneBle(scene, true, true);
							}
							else
							{
								respValue["data"]["code"] = CODE_MEMORY_ERROR;
							}
						}
					}
					for (auto &deviceValue : devices)
					{
						if (deviceValue.isString())
						{
							string deviceId = deviceValue.asString();
							Device *device = getDeviceFromId(deviceId);
							if (device)
							{
								int deviceAddr = device->GetAddr();
								// TODO: them vao phong
								if (room->AddDevice2(device, true))
								{
									database->DeviceInRoomAdd(room, device);
									successList.append(deviceId);
								}
								else
								{
									LOGD("add room deviceId %s error", deviceId.c_str());
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
					respValue["data"]["addr"] = roomAddr;
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
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "createRoomRsp";
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
