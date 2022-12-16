#include "Gateway.h"
#include <Log.h>
#include <unistd.h>
#include <stdio.h>
#include <algorithm>
#include <json.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <thread>
#include "Db.h"
#include "Util.h"
#include "Base64.h"

#include "SceneInputTimer.h"
#include "SceneOutputGroup.h"
#include "SceneOutputDevice.h"

#include "BleProtocol.h"
#include "DeviceBleDownLightSmt.h"
#include "DeviceBleSwitch4.h"
#include "DeviceBleDCSceneContact.h"
#include "DeviceBleTempHumSensor.h"

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#include "DeviceZigbeeOnoff.h"
#include "DeviceZigbeeTelinkOnoff.h"
#endif

#ifndef VERSION
#define VERSION 0.0.1
#endif

#define STR_(x) #x
#define STR(x) STR_(x)

Gateway *gateway = NULL;

Gateway::Gateway(string mac, string server_address, int server_port, string token, string username, string password, int keepalive)
	: CloudProtocol(mac, server_address, server_port, token, username, password, keepalive),
	  LocalProtocol(mac, "localhost", 1883, mac, "", "", 10),
	  Udp(8181)
{
	this->mac = mac;
	dormitoryId = "";
}

void Gateway::init()
{
	CloudProtocol::init();
	LocalProtocol::init();
	Udp::init();

	LOGI("DeviceRead");
	database->DeviceRead();
	database->DeviceAttributeRead();
	database->GroupRead();
	database->DeviceInGroupRead();
	database->SceneRead();

	UdpCmdCallbackRegister("SCAN_HC", bind(&Gateway::OnUdpScanHc, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("HC_SCAN_WIFI", bind(&Gateway::OnUdpHcScanWifi, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("HC_CONNECT_WIFI", bind(&Gateway::OnUdpHcConnectWifi, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("HC_CONNECT_TO_CLOUD", bind(&Gateway::OnUdpHcConnectCloud, this, placeholders::_1, placeholders::_2));

	OnDeviceRPCCallbackRegister("SCAN", bind(&Gateway::OnRPCBleStartScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("STOP", bind(&Gateway::OnRPCBleStopScan, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("BleResetFactory", bind(&Gateway::OnRPCBleResetFactory, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("BleAddDevice", bind(&Gateway::OnRPCBleAddDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("RESET_NODE", bind(&Gateway::OnRPCBleDelDevice, this, placeholders::_1, placeholders::_2));

	OnDeviceRPCCallbackRegister("CREATE_GROUP", bind(&Gateway::OnRPCAddGroup, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("UpdateGroup", bind(&Gateway::OnRPCUpdateGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DELETE_GROUP", bind(&Gateway::OnRPCDelGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("ADD_DEVICE_TO_GROUP", bind(&Gateway::OnRPCAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("REMOVE_DEVICE_FROM_GROUP", bind(&Gateway::OnRPCDelDeviceFromGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("NEW_DEVICE", bind(&Gateway::OnRPCAddTuyaDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DelAllDevice", bind(&Gateway::OnRPCDelAllDevice, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("GetScanDevice", bind(&Gateway::OnRPCGetScanDevice, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("AddScene", bind(&Gateway::OnRPCAddScene, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("DeleteScene", bind(&Gateway::OnRPCDeleteScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DEVICE", bind(&Gateway::OnRPCControlDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("GROUP", bind(&Gateway::OnRPCControlGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DEVICE_UPDATE", bind(&Gateway::OnRPCUpdateAllTelemetry, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("SSHRemote", bind(&Gateway::OnRPCSSHRemote, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("DEVICE", bind(&Gateway::OnRPCControlDevice, this, placeholders::_1, placeholders::_2));

	thread cloudConnectThread(bind(&CloudProtocol::CloudConnect, this, placeholders::_1), 10);
	cloudConnectThread.detach();

	thread localConnectThread(bind(&LocalProtocol::LocalConnect, this, placeholders::_1), 10);
	localConnectThread.detach();
}

void Gateway::OnCloudConnect(bool isConnected, bool isReconnect)
{
	LOGI("OnCloudConnect: %d", isConnected);
	if (isConnected)
	{
		OnlineHC(mac);
		if (!isReconnect)
		{
			for (const auto &[id, device] : deviceList)
			{
				device->PushAttributes();
			}
		}
	}
}

void Gateway::OnLocalConnect(bool isConnected, bool isReconnect)
{
	LOGI("OnLocalConnect: %d", isConnected);
}

int Gateway::OnUdpScanHc(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpScanHc");
	if (reqValue.isMember("DORMITORY_ID") && reqValue["DORMITORY_ID"].isString())
	{
		string dormitoryId = reqValue["DORMITORY_ID"].asString();
		if (this->dormitoryId != "" && this->dormitoryId != dormitoryId)
		{
			return -1;
		}
		respValue["CMD"] = "HC_RESPONSE";
		respValue["IP"] = Util::GetIP();
		respValue["HOSTNAME"] = "RD_HC_" + mac.substr(mac.size() - 5, 2) + mac.substr(mac.size() - 2, 2);
		respValue["MAC"] = mac;
		respValue["TLS"] = false;
		respValue["MQTT_PORT"] = 1883;
		respValue["VERSION"] = STR(VERSION);
		return 0;
	}
	else
	{
		LOGW("OnUdpScanHc payload: %s error", reqValue.toString().c_str());
	}
	return -1;
}

int Gateway::OnUdpHcScanWifi(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpHcScanWifi");
	Util::ScanWifi(respValue);
	return 10; // respValue as an array
}

int Gateway::OnUdpHcConnectWifi(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpHcConnectWifi");
	if (reqValue.isMember("DORMITORY_ID") && reqValue["DORMITORY_ID"].isString())
	{
		string dormitoryId = reqValue["DORMITORY_ID"].asString();
		if (this->dormitoryId == "")
		{
			if (reqValue.isMember("SSID") && reqValue["SSID"].isString() &&
				reqValue.isMember("PASSWORD") && reqValue["PASSWORD"].isString() &&
				reqValue.isMember("ENCRYPTION") && reqValue["ENCRYPTION"].isString())
			{
				string ssidEnc = reqValue["SSID"].asString();
				string passwordEnc = reqValue["PASSWORD"].asString();
				string encryption = reqValue["ENCRYPTION"].asString();
				LOGD("ssidEnc: %s, passwordEnc: %s, encryptionEnd: %s", ssidEnc.c_str(), passwordEnc.c_str(), encryption.c_str());
				string ssid, password;
				if (macaron::Base64::Decode(ssidEnc, ssid) == "" &&
					macaron::Base64::Decode(passwordEnc, password) == "")
				{
					LOGD("ssid: %s, password: %s, encryption: %s", ssid.c_str(), password.c_str(), encryption.c_str());
					respValue["CMD"] = "HC_CONNECT_STATUS";
					if (Util::ConnectToWifi(ssid, password, encryption) == 0)
					{
						this->dormitoryId = dormitoryId;
						respValue["DATA"] = "CONNECT_WIFI_DONE";
					}
					else
					{
						respValue["DATA"] = "CONNECT_WIFI_ERROR";
					}
					return 0;
				}
				else
				{
					LOGW("OnUdpHcConnectWifi Wifi enctyption data error");
					LOGW("ssidEnc: %s, passwordEnc: %s, encryptionEnd: %s", ssidEnc.c_str(), passwordEnc.c_str(), encryption.c_str());
				}
			}
			else
			{
				LOGW("OnUdpHcConnectWifi payload: %s error", reqValue.toString().c_str());
			}
		}
		else
		{
			LOGW("OnUdpHcConnectWifi this->dormitoryId != \"\"");
		}
	}
	else
	{
		LOGW("OnUdpHcConnectWifi payload: %s error", reqValue.toString().c_str());
	}
	return -1;
}

int Gateway::OnUdpHcConnectCloud(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpHcConnectCloud");
	return 1;
}

int Gateway::OnRPCBleStartScan(Json::Value &reqValue, Json::Value &respValue)
{
	scanDeviceList.clear();
	bleProtocol->StartScan();
	return 1;
}

int Gateway::OnRPCBleStopScan(Json::Value &reqValue, Json::Value &respValue)
{
	bleProtocol->StopScan();
	respValue = reqValue;
	return 0;
}

int Gateway::OnRPCBleResetFactory(Json::Value &reqValue, Json::Value &respValue)
{
	bleProtocol->ResetFactory();
	bleProtocol->init();
	respValue["code"] = 0;
	return 0;
}

// int Gateway::OnRPCBleAddDevice(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("params") && reqValue["params"].isObject())
// 	{
// 		Json::Value dataValue = reqValue["params"];
// 		if ( // dataValue.isMember("id") && dataValue["id"].isString() &&
// 				dataValue.isMember("name") && dataValue["name"].isString() &&
// 				dataValue.isMember("mac") && dataValue["mac"].isString() &&
// 				dataValue.isMember("type") && dataValue["type"].isInt64())
// 		{
// 			string deviceId = ""; // dataValue["id"].asString();
// 			string name = dataValue["name"].asString();
// 			string mac = dataValue["mac"].asString();
// 			uint32_t type = dataValue["type"].asInt64();
// 			int rs = bleProtocol->AddDevice(deviceId, name, mac, type);
// 			respValue["code"] = rs;
// 			return 0;
// 		}
// 	}
// 	respValue["code"] = -1;
// 	return -1;
// }

int Gateway::OnRPCBleDelDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCBleDelDevice");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isArray())
	{
		Json::Value dataValue = reqValue["DATA"];
		for (Json::Value::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			string deviceId = dataValue[i].asString();
			LOGD("remove deviceId: %s", deviceId.c_str());
		}
	}
	else
	{
		LOGW("Format error");
	}
	return 0;
}

int Gateway::OnRPCAddScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddScene");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		Scene *scene = AddScene(dataValue, true, true);
		if (scene)
		{
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCDeleteScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDeleteScene");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("id") && dataValue["id"].isInt())
		{
			int sceneId = dataValue["id"].asInt();
			LOGI("Delete Scene id: %d", sceneId);
			delete sceneList[sceneId];
			sceneList.erase(sceneList.find(sceneId));
			database->SceneDel(sceneId);
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCAddGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddGroup");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
			dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray() &&
			dataValue.isMember("NAME") && dataValue["NAME"].isArray())
		{
			string groupId = dataValue["GROUP_ID"].asString();
			string groupName = dataValue["NAME"].asString();
			int temp_groupUnicastId = 0;
			for (auto &x : groupList)
			{
				if (x.first >= temp_groupUnicastId)
				{
					temp_groupUnicastId = x.first;
				}
			}
			Group *group = new Group(groupId, temp_groupUnicastId, groupName);
			if (group)
			{
				group = AddNewGroup(group, true, true);
				if (group)
				{
					Json::Value deviceList = dataValue["DEVICES"].isArray();
					for (int i = 0; i < (int)deviceList.size(); i++)
					{
						string devcieId = deviceList[i].asString();
						Device *device = getDeviceFromId(devcieId);
						if (group && device)
						{
							int tempDeviceAddr = device->GetAddr();
							if (group->AddDevice(device, tempDeviceAddr))
							{
								database->DeviceInGroupAdd(group, device, tempDeviceAddr);
								respValue["code"] = 0;
								return 0;
							}
						}
					}
					respValue["code"] = 0;
					return 0;
				}
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCUpdateGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCUpdateGroup");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("id") && dataValue["id"].isInt() &&
			dataValue.isMember("name") && dataValue["name"].isString())
		{
			int groupId = dataValue["id"].asInt();
			string name = dataValue["name"].asString();
			Group *group = getGroup(groupId);
			if (group)
			{
				group->SetName(name);
				database->GroupUpdate(group);
				respValue["code"] = 0;
				return 0;
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCDelGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDelGroup");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isInt())
		{
			string groupId = dataValue["GROUP_ID"].asString();
			LOGI("Delete Scene id: %s", groupId.c_str());
			Group *group = getGroupFromId(groupId);
			int temp_groupUnicastId = group->GetId();
			delete groupList[temp_groupUnicastId];
			groupList.erase(groupList.find(temp_groupUnicastId));
			for (size_t i = 0; i < group->deviceList.size(); i++)
			{
				group->DelDevice(group->deviceList[i]->device, group->deviceList[i]->epId);
			}
			database->GroupDel(temp_groupUnicastId);
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddDeviceToGroup");
	try
	{
		if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
		{
			Json::Value dataValue = reqValue["DATA"];
			if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
				dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
			{
				string groupId = dataValue["GROUP_ID"].asString();
				Json::Value deviceList = dataValue["DEVICES"].isArray();
				Group *group = getGroupFromId(groupId);
				for (int i = 0; i < (int)deviceList.size(); i++)
				{
					string devcieId = deviceList[i].asString();
					Device *device = getDeviceFromId(devcieId);
					if (group && device)
					{
						int tempDeviceAddr = device->GetAddr();
						if (group->AddDevice(device, tempDeviceAddr))
						{
							database->DeviceInGroupAdd(group, device, tempDeviceAddr);
							respValue["code"] = 0;
							return 0;
						}
					}
				}
			}
		}
		respValue["code"] = -1;
		return -1;
	}
	catch (const char *msg)
	{
		LOGE("OnRPCAddDeviceToGroup fail");
		return -1;
	}
}

/**
 * @brief delete device from group
 *
 * @param [in] reqValue json input
 * @param [out] respValue json output
 * @return int -1 - error, 0 - success
 */
int Gateway::OnRPCDelDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDelDeviceFromGroup");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
			dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
		{
			int groupId = dataValue["GROUP_ID"].asInt();
			Group *group = getGroup(groupId);
			Json::Value deviceList = dataValue["DEVICES"];
			for (int i = 0; i < (int)deviceList.size(); i++)
			{
				string deviceId = deviceList[i].asString();
				Device *device = getDeviceFromId(deviceId);
				if (group && device)
				{
					int tempDeviceAddr = device->GetAddr();
					if (group->AddDevice(device, tempDeviceAddr))
					{
						database->DeviceInGroupDel(group, device, tempDeviceAddr);
						respValue["code"] = 0;
						return 0;
					}
				}
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

/**
 * @brief
 *
 * @param reqValue
 * @param respValue
 * @return int
 */
int Gateway::OnRPCAddDevice(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("id") && dataValue["id"].isString() &&
			dataValue.isMember("name") && dataValue["name"].isString() &&
			dataValue.isMember("mac") && dataValue["mac"].isString() &&
			dataValue.isMember("addr") && dataValue["addr"].isInt() &&
			dataValue.isMember("type") && dataValue["type"].isInt())
		{
			string deviceId = dataValue["id"].asString();
			string name = dataValue["name"].asString();
			string mac = dataValue["mac"].asString();
			uint32_t addr = dataValue["addr"].asInt();
			uint32_t type = dataValue["type"].asInt();
			Device *device = AddNewDevice(deviceId, name, mac, addr, type, true, true);
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCAddTuyaDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddTuyaDevice");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() &&
			dataValue.isMember("PROPERTIES") && dataValue["PROPERTIES"].isArray())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			Json::Value properties = dataValue["PROPERTIES"];
			LOGI("New Tuya Device: %s", deviceId.c_str());
			// Device *device = getDeviceFromId(deviceId);
			// if (device)
			// {
			// 	for (Json::Value::ArrayIndex i = 0; i < properties.size(); i++)
			// 	{
			// 		Json::Value property = properties[i];
			// 		if (property.isObject() &&
			// 				property.isMember("ID") && property["ID"].isInt() &&
			// 				property.isMember("CMD") && property["CMD"].isString())
			// 		{
			// 			int id = property["ID"].asInt();
			// 			string cmd = property["CMD"].asString();
			// 			device->Do(id, value);
			// 		}
			// 	}
			// }
			// else
			// {
			// 	LOGW("Device not found");
			// }
		}
		else
		{
			LOGW("Format error");
		}
	}
	else
	{
		LOGW("Format error");
	}
	respValue = reqValue;
	return 0;
}

//
int Gateway::OnRPCDelAllDevice(Json::Value &reqValue, Json::Value &respValue)
{
	database->DeviceDelAll();
	deviceList.clear();
	bleProtocol->ResetFactory();
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCGetScanDevice(Json::Value &reqValue, Json::Value &respValue)
{
	Json::Value scanDeviceValues;
	for (auto &scanDevice : scanDeviceList)
	{
		Json::Value scanDeviceValue;
		scanDeviceValue["name"] = scanDevice->GetName();
		scanDeviceValue["mac"] = scanDevice->GetMac();
		scanDeviceValue["type"] = scanDevice->GetType();
		scanDeviceValue["version"] = scanDevice->GetVersion();
		scanDeviceValue["rssi"] = scanDevice->GetRSSI();
		scanDeviceValues.append(scanDeviceValue);
		// delete scanDevice;
	}
	respValue["devices"] = scanDeviceValues;
	respValue["count"] = scanDeviceList.size();
	respValue["code"] = 0;
	// scanDeviceList.clear();
	Json::Value jsonValue;
	jsonValue["HaveNewDevice"] = false;
	PublishToDeviceTelemetry(jsonValue);
	return 0;
}

int Gateway::OnRPCControlDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCControlDevice");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() &&
			dataValue.isMember("PROPERTIES") && dataValue["PROPERTIES"].isArray())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			Json::Value properties = dataValue["PROPERTIES"];
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				for (Json::Value::ArrayIndex i = 0; i < properties.size(); i++)
				{
					Json::Value property = properties[i];
					if (property.isObject() &&
						property.isMember("ID") && property["ID"].isInt() &&
						property.isMember("VALUE") && property["VALUE"].isInt())
					{
						int id = property["ID"].asInt();
						int value = property["VALUE"].asInt();
						device->Do(id, value);
					}
				}
			}
			else
			{
				LOGW("Device not found");
			}
		}
		else
		{
			LOGW("Format error");
		}
	}
	else
	{
		LOGW("Format error");
	}
	respValue = reqValue;
	return 1;
}

int Gateway::OnRPCControlGroup(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
			dataValue.isMember("PROPERTIES") && dataValue["PROPERTIES"].isArray())
		{
			string groupId = dataValue["GROUP_ID"].asString();
			Json::Value properties = dataValue["PROPERTIES"];
			Group *group = getGroupFromId(groupId);
			if (group)
			{
				for (Json::Value::ArrayIndex i = 0; i < properties.size(); i++)
				{
					Json::Value property = properties[i];
					if (property.isObject() &&
						property.isMember("ID") && property["ID"].isInt() &&
						property.isMember("VALUE") && property["VALUE"].isInt())
					{
						int id = property["ID"].asInt();
						int value = property["VALUE"].asInt();
						group->Do(id, value);
					}
				}
			}
			else
			{
				LOGW("Group not found");
			}
		}
		else
		{
			LOGW("Format error");
		}
	}
	else
	{
		LOGW("Format error");
	}
	return 0;
}

int Gateway::OnRPCUpdateAllTelemetry(Json::Value &reqValue, Json::Value &respValue)
{
	Json::Value dataValue;
	Json::Value deviceValue;
	Json::Value propertiesValue;
	for (const auto &[id, device] : deviceList)
	{
		propertiesValue = Json::Value::null;
		deviceValue = Json::Value::null;
		device->BuildTelemetryValue(propertiesValue);
		deviceValue["DEVICE_ID"] = device->GetId();
		deviceValue["PROPERTIES"] = propertiesValue;
		dataValue.append(deviceValue);
	}
	respValue["CMD"] = "DEVICE_UPDATE";
	respValue["DATA"] = dataValue;
	return 0;
}

int Gateway::OnRPCSSHRemote(Json::Value &reqValue, Json::Value &respValue)
{
	int rs = 0;
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("type") && dataValue["type"].isString() &&
			dataValue.isMember("key") && dataValue["key"].isString() &&
			dataValue.isMember("user") && dataValue["user"].isString() &&
			dataValue.isMember("host") && dataValue["host"].isString() &&
			dataValue.isMember("serverPort") && dataValue["serverPort"].isInt() &&
			dataValue.isMember("forwardPort") && dataValue["forwardPort"].isInt())
		{
			string key = "";
			string type = dataValue["type"].asString();
			string user = dataValue["user"].asString();
			string host = dataValue["host"].asString();
			uint32_t serverPort = dataValue["serverPort"].asInt();
			uint32_t forwardPort = dataValue["forwardPort"].asInt();
			uint32_t localPort = 22;
			if (dataValue.isMember("localPort") && dataValue["localPort"].isInt())
			{
				localPort = dataValue["localPort"].asInt();
			}
			if (type == "base64")
			{
				string keyBase64 = dataValue["key"].asString();
				string encode = macaron::Base64::Decode(keyBase64, key);
				if (encode != "")
				{
					rs = 1;
				}
			}
			else
			{
				key = dataValue["key"].asString();
			}

			if (rs == 0)
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
				return 0;
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

void Gateway::AddDeviceToScanList(Device *scanDevice)
{
	Json::Value jsonValue;
	Json::Value dataValue;
	if (!scanDevice)
	{
		LOGW("scanDevice null");
		return;
	}
	dataValue["DEVICE_ID"] = scanDevice->GetId();
	dataValue["DEVICE_UNICAST_ID"] = scanDevice->GetAddr();
	dataValue["DEVICE_TYPE_ID"] = scanDevice->GetType();
	dataValue["MAC_ADDRESS"] = scanDevice->GetMac();
	dataValue["FIRMWARE_VERSION"] = scanDevice->GetVersionStr();
	dataValue["DEVICE_KEY"] = scanDevice->GetId();
	dataValue["NET_KEY"] = scanDevice->GetId();
	dataValue["APP_KEY"] = scanDevice->GetId();
	jsonValue["CMD"] = "NEW_DEVICE";
	jsonValue["DATA"] = dataValue;
	PublishToDeviceTelemetry(jsonValue);
}

Group *Gateway::getGroup(int id)
{
	if (groupList.find(id) != groupList.end())
	{
		return groupList[id];
	}
	return NULL;
}

Group *Gateway::getGroupFromId(string groupId)
{
	for (const auto &[id, group] : groupList)
	{
		if (group->GetName() == groupId)
			return group;
	}
	return NULL;
}

Device *Gateway::getDevice(string mac)
{
	if (deviceList.find(mac) != deviceList.end())
	{
		return deviceList[mac];
	}
	return NULL;
}

Device *Gateway::getDeviceFromId(string deviceId)
{
	for (const auto &[id, device] : deviceList)
	{
		if (device->GetId() == deviceId)
			return device;
	}
	return NULL;
}

DeviceBle *Gateway::getDeviceBleFromAddr(uint32_t addr)
{
	for (const auto &[id, device] : deviceList)
	{
		if (device->CheckAddr(addr) && device->GetProtocol() >= BLE_DEVICE)
		{
			DeviceBle *deviceBle = dynamic_cast<DeviceBle *>(device);
			if (deviceBle)
				return deviceBle;
		}
	}
	return NULL;
}

#ifdef CONFIG_ENABLE_ZIGBEE
DeviceZigbee *Gateway::getDeviceZigbeeFromAddr(uint32_t addr)
{
	for (const auto &[id, device] : deviceList)
	{
		if (device->CheckAddr(addr) && device->GetProtocol() >= ZIGBEE_DEVICE)
		{
			DeviceZigbee *deviceZigbee = dynamic_cast<DeviceZigbee *>(device);
			if (deviceZigbee)
				return deviceZigbee;
		}
	}
	return NULL;
}
#endif

Device *Gateway::AddNewDevice(string id, string name, string mac, uint32_t addr, uint32_t type, bool addGateway, bool addDatabase)
{
	LOGI("Add new device id: %s, name: %s, mac: %s, addr: 0x%04X, type: 0x%04X", id.c_str(), name.c_str(), mac.c_str(), addr, type);
	Device *device = NULL;
	if (type == BLE_DOWNLIGHT_SMT)
	{
		device = new DeviceBleDownLightSmt(id, name, mac, addr);
	}
	else if (type == BLE_SWITCH_4)
	{
		device = new DeviceBleSwitch4(id, name, mac, addr);
	}
	else if (type == BLE_DC_SCENE_CONTACT)
	{
		device = new DeviceBleDCSceneContact(id, name, mac, addr);
	}
	else if (type == BLE_TEMP_HUM_SENSOR)
	{
		device = new DeviceBleTempHumSensor(id, name, mac, addr);
	}
#ifdef CONFIG_ENABLE_ZIGBEE
	if (type == ZIGBEE_LUMI_PLUG)
	{
		device = new DeviceZigbeeOnoff(id, name, mac, addr);
	}
	else if (type == ZIGBEE_TELINK_TLSR82xx)
	{
		device = new DeviceZigbeeTelinkOnoff(id, name, mac, addr);
	}
#endif

	if (device)
	{
		if (addGateway)
			deviceList[mac] = device;
		if (addDatabase)
			database->DeviceAdd(device);
		// if (connected)
		// 	device->PushAttributes();
	}
	else
	{
		LOGW("Add new device not support type: 0x%04X", type);
	}
	return device;
}

Group *Gateway::AddNewGroup(Group *group, bool addGateway, bool addDatabase)
{
	if (group)
	{
		if (addDatabase)
		{
			int rs = database->GroupAdd(group);
			if (rs)
			{
				LOGW("rs: %d", rs);
				return NULL;
			}
		}
		if (addGateway)
			groupList[group->GetId()] = group;
	}
	return group;
}

Scene *Gateway::AddScene(Json::Value &sceneValue, bool addGateway, bool addDatabase)
{
	// TODO: Check Scene id exist
	if (sceneValue.isMember("id") && sceneValue["id"].isInt() &&
		sceneValue.isMember("repeat") && sceneValue["repeat"].isInt() &&
		sceneValue.isMember("fullDay") && sceneValue["fullDay"].isBool() &&
		sceneValue.isMember("type") && sceneValue["type"].isString() &&
		sceneValue.isMember("input") && sceneValue["input"].isObject() &&
		sceneValue.isMember("output") && sceneValue["output"].isObject())
	{
		int id = sceneValue["id"].asInt();
		int repeat = sceneValue["repeat"].asInt();
		bool fullDay = sceneValue["fullDay"].asBool();
		string type = sceneValue["type"].asString();
		Scene *scene = NULL;
		if (!fullDay && sceneValue.isMember("startTime") && sceneValue["startTime"].isString() &&
			sceneValue.isMember("endTime") && sceneValue["endTime"].isString())
		{
			string startTime = sceneValue["startTime"].asString();
			string endTime = sceneValue["endTime"].asString();
			scene = new Scene(id, type, repeat, Util::ConvertStrTimeToInt(startTime), Util::ConvertStrTimeToInt(endTime));
		}
		else
		{
			scene = new Scene(id, type, repeat);
		}
		if (!scene)
		{
			LOGE("New scene error");
			return NULL;
		}
		Json::Value inputValue = sceneValue["input"];
		if (inputValue.isMember("timer") && inputValue["timer"].isObject())
		{
			Json::Value timerValue = inputValue["timer"];
			if (timerValue.isMember("repeat") && timerValue["repeat"].isInt() &&
				timerValue.isMember("time") && timerValue["time"].isString())
			{
				int repeat = timerValue["repeat"].asInt();
				string timerStr = timerValue["time"].asString();
				LOGI("Have Timer: %s", timerStr.c_str());
				int timer = Util::ConvertStrTimeToInt(timerStr);
				if (timer > 0)
				{
					SceneInputTimer *sceneInputTimer = new SceneInputTimer(scene, timer, repeat);
					scene->AddSceneInput(sceneInputTimer);
				}
			}
		}
		if (inputValue.isMember("device") && inputValue["device"].isArray())
		{
			Json::Value deviceSceneInputList = inputValue["device"];
			for (Json::Value::ArrayIndex i = 0; i < deviceSceneInputList.size(); i++)
			{
				Json::Value deviceSceneInputValue = deviceSceneInputList[i];
				if (deviceSceneInputValue.isObject())
				{
					if (deviceSceneInputValue.isMember("mac") && deviceSceneInputValue["mac"].isString() &&
						deviceSceneInputValue.isMember("data") && deviceSceneInputValue["data"].isObject())
					{
						string mac = deviceSceneInputValue["mac"].asString();
						Json::Value dataValue = deviceSceneInputValue["data"];
						Device *device = getDevice(mac);
						if (device)
						{
							SceneInputDevice *sceneInputDevice = new SceneInputDevice(scene, device, dataValue);
							scene->AddSceneInput(sceneInputDevice);
						}
					}
				}
			}
		}

		Json::Value outputValue = sceneValue["output"];
		if (outputValue.isMember("device") && outputValue["device"].isArray())
		{
			Json::Value deviceSceneOutputList = outputValue["device"];
			for (Json::Value::ArrayIndex i = 0; i < deviceSceneOutputList.size(); i++)
			{
				Json::Value deviceSceneOutputValue = deviceSceneOutputList[i];
				if (deviceSceneOutputValue.isObject())
				{
					if (deviceSceneOutputValue.isMember("mac") && deviceSceneOutputValue["mac"].isString() &&
						deviceSceneOutputValue.isMember("data") && deviceSceneOutputValue["data"].isObject())
					{
						Json::Value dataValue = deviceSceneOutputValue["data"];
						string mac = deviceSceneOutputValue["mac"].asString();
						Device *device = getDevice(mac);
						if (device)
						{
							SceneOutputDevice *sceneOutputDevice = new SceneOutputDevice(device, dataValue);
							scene->AddSceneOutput(sceneOutputDevice);
						}
					}
				}
			}
		}
		if (outputValue.isMember("group") && outputValue["group"].isArray())
		{
			Json::Value groupSceneOutputList = outputValue["group"];
			for (Json::Value::ArrayIndex i = 0; i < groupSceneOutputList.size(); i++)
			{
				Json::Value groupSceneOutputValue = groupSceneOutputList[i];
				if (groupSceneOutputValue.isObject())
				{
					if (groupSceneOutputValue.isMember("id") && groupSceneOutputValue["id"].isInt() &&
						groupSceneOutputValue.isMember("data") && groupSceneOutputValue["data"].isObject())
					{
						int id = groupSceneOutputValue["id"].asInt();
						Json::Value dataValue = groupSceneOutputValue["data"];
						Group *group = getGroup(id);
						if (group)
						{
							SceneOutputGroup *sceneOutputGroup = new SceneOutputGroup(group, dataValue);
							scene->AddSceneOutput(sceneOutputGroup);
						}
					}
				}
			}
		}
		LOGI("Add Scene %d", scene->GetId());
		if (addGateway)
			sceneList[scene->GetId()] = scene;
		if (addDatabase)
		{
			string sceneStr = sceneValue.toString();
			sceneStr.erase(remove_if(sceneStr.begin(), sceneStr.end(), ::isspace), sceneStr.end());
			database->SceneAdd(scene->GetId(), sceneStr);
		}
		scene->Check();
		return scene;
	}
	else
	{
		LOGW("Scene format error");
	}
	return NULL;
}
