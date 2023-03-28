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
	OnDeviceRpcCallbackRegisterV2("controlGroup", bind(&Gateway::OnControlGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("getDevStt", bind(&Gateway::OnGetDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("getAllDevStt", bind(&Gateway::OnGetAllDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("getDevList", bind(&Gateway::OnGetDeviceList, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("getHcInfo", bind(&Gateway::OnGetHcInfo, this, placeholders::_1, placeholders::_2));
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
	OnDeviceRpcCallbackRegisterV2("delRule", bind(&Gateway::OnDeleteRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("addDevToRoom", bind(&Gateway::OnAddDeviceToRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("delDevFromRoom", bind(&Gateway::OnDeleteDeviceFromRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("delRoom", bind(&Gateway::OnDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("SSHRemote", bind(&Gateway::OnSSHRemote, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
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
	OnLocalCallbackRegisterV2("addDevToRoom", bind(&Gateway::OnAddDeviceToRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delDevFromRoom", bind(&Gateway::OnDeleteDeviceFromRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("delRoom", bind(&Gateway::OnDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("SSHRemote", bind(&Gateway::OnSSHRemote, this, placeholders::_1, placeholders::_2));
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
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value dataValue = reqValue["data"];
		if (dataValue.isObject())
		{
			if (bleProtocol)
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
				respValue["data"]["code"] = CODE_OK;
			}
			if (dataValue.isMember(KEY_ATTRIBUTE_DIM) && dataValue[KEY_ATTRIBUTE_DIM].isInt())
			{
				LOGW("BleProtocol null");
				respValue["data"]["code"] = CODE_ERROR;
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
	respValue["cmd"] = "controlAllDevRsp";
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
					device->BuildTelemetryValueV2(deviceValue);
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
	respValue["cmd"] = "getDevSttRsp";
	return CODE_OK;
}

int Gateway::OnGetAllDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetAllDeviceStatus");
	Json::Value devicesData;
	AddAllDeviceStatusV2(devicesData);
	respValue["data"]["code"] = CODE_OK;
	respValue["data"]["devices"] = devicesData;
	respValue["cmd"] = "getAllDevSttRsp";
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
		deviceValue["addr"] = device->GetAddr();
		deviceValue["type"] = device->GetType();
		deviceValue["mac"] = device->GetMac();
		deviceValue["ver"] = device->GetVersionStr();
		devicesData.append(deviceValue);
	}
	respValue["data"]["devices"] = devicesData;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getDeviceList";
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
	respValue["cmd"] = "getInfoHcRsp";
	return CODE_OK;
}

int Gateway::OnStartScanBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (bleProtocol)
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
	else
		LOGW("BleProtocol null");
	return CODE_ERROR;
}

int Gateway::OnStopScanBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (bleProtocol)
	{
		bleProtocol->StopScan();
		bleProtocol->isAdding = false;
		bleProtocol->isProvisioning = false;
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
	if (reqValue.isMember("devices") && reqValue["devices"].isArray())
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
					if (bleProtocol)
					{
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
	respValue["cmd"] = "addDevToGroupRsp";
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
				Json::Value groupList = reqValue["devices"];
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
								string deviceId = deviceValue.asString();
								Device *device = getDeviceFromId(deviceId);
								if (device)
								{
									if (scene->AddDeviceV2(device, deviceProperties, false))
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
				if (scene->DelDevice(deviceInScene->device))
				{
					database->DeviceInSceneBleDel(scene, deviceInScene->device);
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
		ruleList[rule->GetId()] = rule;
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
			database->RuleDel(rule);
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
