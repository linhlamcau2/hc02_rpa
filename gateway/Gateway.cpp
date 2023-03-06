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
#include "Wifi.h"
#include "Ota.h"
#include "Base64.h"
#ifndef ESP_PLATFORM
#include "Config.h"
#endif
#include "Http.h"
#include "File.h"

#include "RuleInputTimer.h"
#include "RuleOutputGroup.h"
#include "RuleOutputDevice.h"

#include "BleDefine.h"
#include "BleProtocol.h"
#include "DeviceBleLightOnoffCctDim.h"
#include "DeviceBleLightOnoffHslModeRGB.h"
#include "DeviceBleLightOnoffCctDimHslModeRGB.h"
#include "DeviceBleSwitchTouchRgb4.h"
#include "DeviceBleSwitchScene6DC.h"
#include "DeviceBleSensorTempHum.h"
#include "DeviceBleSensorPm.h"
#include "DeviceBlePirLightSensorDC.h"
#include "DeviceBleSmokeSensor.h"
#include "DeviceBleDoorSensor.h"
#include "DeviceBleScreenTouch.h"

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

Gateway::Gateway(string mac, string server_address, int server_port, string token, string username, string password, int keepalive, string localIp, int localPort, string localUsername, string localPassword, int localKeepalive)
		: CloudProtocol(mac, server_address, server_port, token, username, password, keepalive),
			LocalProtocol(mac, localIp, localPort, mac, localUsername, localPassword, localKeepalive),
			Udp(8181)
{
	this->mac = mac;
	this->id = "";
	this->dormitoryId = "";
	this->refresh_token = "";
	this->ble_unicast = 0;
	this->ble_appkey = "";
	this->ble_appkey = "";
	this->ble_devicekey = "";
	udpBroadcastThread = NULL;
}

void Gateway::init()
{
	CloudProtocol::init();
	LocalProtocol::init();
	Udp::init();

	LOGI("DeviceRead");
	database->GatewayRead();
	database->DeviceRead();
	database->DeviceAttributeRead();
	database->GroupRead();
	database->DeviceInGroupRead();
	database->RuleRead();
	database->SceneBleRead();
	database->RoomRead();
	database->DataRoomRead();
	database->DeviceInRoomRead();
	for (auto it = roomList.begin(); it != roomList.end(); it++)
	{
		cout << "***" << it->first.c_str() << endl;
	}
	if (gateway->getId().compare("") == 0)
	{
		id = mac;
		gateway->setId(id);
		database->GatewayUpdateId(gateway, id);
		database->GatewayRead();
	}

	UdpCmdCallbackRegister("SCAN_HC", bind(&Gateway::OnUdpScanHc, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("HC_SCAN_WIFI", bind(&Gateway::OnUdpHcScanWifi, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("SETUP_HC", bind(&Gateway::OnUdpHcSetup, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("HC_CONNECT_TO_CLOUD", bind(&Gateway::OnUdpHcConnectCloud, this, placeholders::_1, placeholders::_2));

	OnDeviceRPCCallbackRegister("HC_CONNECT_TO_CLOUD", bind(&Gateway::OnRPCHcConnectCloud, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("HC_BACKUP_DATA", bind(&Gateway::OnRPCHcBackup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("SCAN", bind(&Gateway::OnRPCBleStartScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("STOP", bind(&Gateway::OnRPCBleStopScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("RESET_NODE", bind(&Gateway::OnRPCBleDelDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("RESET_BLE", bind(&Gateway::OnRPCBleReset, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("RESET_HC", bind(&Gateway::OnRPCResetFactory, this, placeholders::_1, placeholders::_2));

	OnDeviceRPCCallbackRegister("CREATE_GROUP", bind(&Gateway::OnRPCAddGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DELETE_GROUP", bind(&Gateway::OnRPCDelGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("ADD_DEVICE_TO_GROUP", bind(&Gateway::OnRPCAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DELETE_DEVICE_FROM_GROUP", bind(&Gateway::OnRPCDelDeviceFromGroup, this, placeholders::_1, placeholders::_2));

	OnDeviceRPCCallbackRegister("CREATE_SCENE", bind(&Gateway::OnRPCAddSceneBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("EDIT_SCENE", bind(&Gateway::OnRPCEditSceneBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DELETE_SCENE", bind(&Gateway::OnRPCDeleteSceneBle, this, placeholders::_1, placeholders::_2));

	OnDeviceRPCCallbackRegister("NEW_DEVICE", bind(&Gateway::OnRPCAddTuyaDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DelAllDevice", bind(&Gateway::OnRPCDelAllDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DEVICE", bind(&Gateway::OnRPCControlDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("GROUP", bind(&Gateway::OnRPCControlGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("SCENE", bind(&Gateway::OnRPCControlSceneBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DEVICE_UPDATE", bind(&Gateway::OnRPCUpdateAllTelemetry, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("SSHRemote", bind(&Gateway::OnRPCSSHRemote, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("HC_CONNECT_TO_CLOUD", bind(&Gateway::OnRPCHcConnectCloud, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("HC_BACKUP_DATA", bind(&Gateway::OnRPCHcBackup, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("SCAN", bind(&Gateway::OnRPCBleStartScan, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("STOP", bind(&Gateway::OnRPCBleStopScan, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("RESET_NODE", bind(&Gateway::OnRPCBleDelDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("RESET_BLE", bind(&Gateway::OnRPCBleReset, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("RESET_HC", bind(&Gateway::OnRPCResetFactory, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_ROOM", bind(&Gateway::OnRPCCreateRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("ADD_DEVICE_TO_ROOM", bind(&Gateway::OnRPCAddDevToRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("REMOVE_DEVICE_FROM_ROOM", bind(&Gateway::OnRPCRemoveDevFromRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_ROOM", bind(&Gateway::OnRPCDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("CHECK_ROOM", bind(&Gateway::OnRPCCheckRoom, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_GROUP", bind(&Gateway::OnRPCAddGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_GROUP", bind(&Gateway::OnRPCDelGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("ADD_DEVICE_TO_GROUP", bind(&Gateway::OnRPCAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_DEVICE_FROM_GROUP", bind(&Gateway::OnRPCDelDeviceFromGroup, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_SCENE", bind(&Gateway::OnRPCAddSceneBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EDIT_SCENE", bind(&Gateway::OnRPCEditSceneBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_SCENE", bind(&Gateway::OnRPCDeleteSceneBle, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("NEW_DEVICE", bind(&Gateway::OnRPCAddTuyaDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DelAllDevice", bind(&Gateway::OnRPCDelAllDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DEVICE", bind(&Gateway::OnRPCControlDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("GROUP", bind(&Gateway::OnRPCControlGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("SCENE", bind(&Gateway::OnRPCControlSceneBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DEVICE_UPDATE", bind(&Gateway::OnRPCUpdateAllTelemetry, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("SSHRemote", bind(&Gateway::OnRPCSSHRemote, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("UPDATE_FIRMWARE", bind(&Gateway::OnRPCUpdateFirmware, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("SCENE_FOR_REMOTE", bind(&Gateway::OnRPCSetSceneForRemote, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_SCENE_FOR_REMOTE", bind(&Gateway::OnRPCDelSceneForRemote, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("RESET_REMOTE", bind(&Gateway::OnRPCResetRemote, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("SCENE_FOR_SENSOR_LIGHT_PIR", bind(&Gateway::OnRPCScenePirLigtSensor, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EDIT_SCENE_FOR_SENSOR_LIGHT_PIR", bind(&Gateway::OnRPCScenePirLigtSensor, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("REMOVE_SCENE_FOR_SENSOR_LIGHT_PIR", bind(&Gateway::OnRPCRemoveScenePirLightSensor, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("SCENE_FOR_SCREEN", bind(&Gateway::OnRPCSceneScreen, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_EVENT_TRIGGER", bind(&Gateway::OnRPCAddRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EDIT_EVENT_TRIGGER", bind(&Gateway::OnRPCEditRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EVENT_TRIGGER_STATUS", bind(&Gateway::OnRPCSwitchStatusEvent, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_EVENT_TRIGGER", bind(&Gateway::OnRPCDeleteRule, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("COUNTDOWN", bind(&Gateway::OnRPCCreateCountDown, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_COUNTDOWN", bind(&Gateway::OnRPCDelCountDown, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_HCL", bind(&Gateway::OnRPCCreateHCL, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EDIT_HCL", bind(&Gateway::OnRPCEditHCL, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("HCL_RULE_STATUS", bind(&Gateway::OnRPCSwitchStatusEvent, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("SET_PASSWD_MQTT_ONLINE", bind(&Gateway::OnRPCSetPwMqttOnline, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("ADD_DEVICE_SMARTHOME_TO_ROOM", bind(&Gateway::OnRPCAddDeviceSmartHomeToRoom, this, placeholders::_1, placeholders::_2));

	CloudConnect();
	LocalConnect();

	thread checkOnlineThread(bind(&Gateway::CheckOnlineThread, this));
	checkOnlineThread.detach();
}

void Gateway::OnCloudConnect(bool isConnected, bool isReconnect)
{
	LOGI("OnCloudConnect: %d", isConnected);
	if (isConnected)
	{
		Util::LedInternet(true);
		OnlineHC(mac);
		if (!isReconnect)
		{
			for (const auto &[id, device] : deviceList)
			{
				device->PushAttributes();
			}
		}

		file->uploadFile(".", "smh.sqlite");
	}
	else
	{
		Util::LedInternet(false);
	}
}

void Gateway::OnLocalConnect(bool isConnected, bool isReconnect)
{
	LOGI("OnLocalConnect: %d", isConnected);
}

void Gateway::ResetFactory()
{
	LOGI("ResetFactory");
	deviceList.clear();
	groupList.clear();
	ruleList.clear();
	sceneBleList.clear();
	scanDeviceList.clear();

	database->DeviceDelAll();
	database->GatewayDelAll();
	database->GatewayUpdateId(gateway, gateway->getId());
	gateway->setBleAppkey("");
	database->DeviceAttributeDelAll();
	database->GroupDelAll();
	database->DeviceInGroupDelAll();
	bleProtocol->ResetDelAll();
	bleProtocol->ResetFactory();
}

int Gateway::CheckOnlineThread()
{
	LOGI("Start CheckOnlineThread");
	time_t currentTime = 0;
	uint32_t allTimeCheck = 0; // time total in a loop check
	bool deviceStateChange = false;
	Json::Value onlineValue;
	Json::Value datasValue;
	Json::Value dataValue;
	Json::Value propertysValue;
	Json::Value propertyValue;
	propertyValue["ID"] = BLE_ATTRIBUTE_ONLINE_OFFLINE;
	propertyValue["VALUE"] = 1;
	propertysValue.append(propertyValue);
	dataValue["DEVICE_ID"] = "";
	dataValue["PROPERTIES"] = propertysValue;
	datasValue.append(dataValue);
	onlineValue["CMD"] = "DEVICE";
	onlineValue["DATA"] = datasValue;

	while (1)
	{
		currentTime = time(NULL);
		allTimeCheck = deviceList.size() * 4;
		for (const auto &[id, device] : deviceList)
		{
			deviceStateChange = false;
			if (device->lastOnlineState) // online
			{
				// neu thiet bi ho tro ban tin check trang thai online/offline
				if (device->isNeedCheckOnline())
				{
					// thoi gian lan cuoi cung nhan ban tin hoac lan cuoi cung check qua 1 chu ky
					if ((device->lastTimeActive + allTimeCheck) <= currentTime && (device->lastTimeCheck + allTimeCheck) <= currentTime)
					{
						bleProtocol->SendOnlineCheck(device->GetAddr());
						device->lastTimeCheck = currentTime;
					}
					// 2 chu ky khong co ban tin phan hoi thi bao offline
					if ((device->lastTimeActive + allTimeCheck * 2) < currentTime)
					{
						LOGI("Device 0x%04X offline", device->GetAddr());
						device->lastOnlineState = false;
						deviceStateChange = true;
					}
				}
				// neu thiet bi khong ho tro ban tin check trang thai online/offline
				else
				{
					// 1 ngay khong co ban tin moi thi bao offline
					if ((device->lastTimeActive + 60 * 60 * 24) < currentTime)
					{
						LOGI("Device 0x%04X offline", device->GetAddr());
						device->lastOnlineState = false;
						deviceStateChange = true;
					}
				}
			}
			else
			{
				if (device->isNeedCheckOnline())
				{
					// thoi gian check qua 1 chu ky thi check lai
					if ((device->lastTimeCheck + allTimeCheck) <= currentTime)
					{
						bleProtocol->SendOnlineCheck(device->GetAddr());
						device->lastTimeCheck = currentTime;
					}
					// neu co ban tin moi trong vong 2 chu ky check thi bao online
					if ((device->lastTimeActive + allTimeCheck * 2) >= currentTime)
					{
						LOGI("Device 0x%04X online", device->GetAddr());
						device->lastOnlineState = true;
						deviceStateChange = true;
					}
				}
				else
				{
					// trong ngay co ban tin thi online
					if ((device->lastTimeActive + 60 * 60 * 24) >= currentTime)
					{
						LOGI("Device 0x%04X online", device->GetAddr());
						device->lastOnlineState = true;
						deviceStateChange = true;
					}
				}
			}
			// send device state to server
			if (deviceStateChange)
			{
				onlineValue["DATA"][0]["DEVICE_ID"] = device->GetId();
				onlineValue["DATA"][0]["PROPERTIES"][0]["VALUE"] = (int)device->lastOnlineState;
				PublishToLocalMessage(onlineValue);
				PublishToGatewayTelemetry(onlineValue);
			}
		}
		sleep(1);
	}
	return 0;
}

int Gateway::UdpBroadcastThread()
{
	LOGI("Start UdpBroadcastThread");
	struct sockaddr_in s;
	memset(&s, 0, sizeof(struct sockaddr_in));
	s.sin_family = AF_INET;
	s.sin_port = htons(8181);
	string ip = Wifi::GetIP();
	LOGD("IP: %s", ip.c_str());
	if (ip.compare("10.10.10.1") == 0)
	{
		inet_pton(AF_INET, "10.10.10.255", &s.sin_addr);
	}
	else
	{
		s.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	}

	Json::Value hcBroadcastValue;
	Json::Value hcInfoValue;
	Json::Value appInfoValaue;
	Json::Value dataValue;
	hcBroadcastValue["CMD"] = "HC_BROADCAST";
	hcBroadcastValue["REQUEST_ID"] = Util::genRandRQI(16);
	hcBroadcastValue["TIME"] = Util::GetCurrentTimeStr();
	hcBroadcastValue["CONNECTION_TYPE"] = 0;
	hcInfoValue["TYPE"] = 2;
	hcInfoValue["DORMITORY_ID"] = dormitoryId;
	hcInfoValue["MAC"] = mac;
	hcInfoValue["VERSION"] = STR(VERSION);
	hcInfoValue["IP"] = Wifi::GetIP();
	hcBroadcastValue["FROM"] = hcInfoValue;
	appInfoValaue["TYPE"] = 0;
	hcBroadcastValue["TO"] = appInfoValaue;
	if (Wifi::GetIP().compare("10.10.10.1") == 0)
	{
		dataValue["CONNECTION_MODE"] = "Wifi";
	}
	else
	{
		dataValue["CONNECTION_MODE"] = "Lan";
	}
	hcBroadcastValue["DATA"] = dataValue;
	isUdpBroadcasting = true;
	bool ledInternet = Util::GetStatusLedInternet();
	for (int i = 0; i < 120; i++)
	{
		if (!isUdpBroadcasting)
			break;
		Util::LedInternet(false);
		usleep(500000);
		send(hcBroadcastValue.toString(), &s, sizeof(s));
		Util::LedInternet(true);
		usleep(500000);
	}
	Util::LedInternet(ledInternet);
	isUdpBroadcasting = false;
	free(udpBroadcastThread);
	udpBroadcastThread = NULL;
	return 0;
}

void Gateway::StartUdpBroadcast()
{
	if (!udpBroadcastThread)
	{
		udpBroadcastThread = new thread(bind(&Gateway::UdpBroadcastThread, this));
		udpBroadcastThread->detach();
	}
	else
	{
		LOGI("StartUdpBroadcast is still running...");
	}
}

void Gateway::StopUdpBroadcast()
{
	isUdpBroadcasting = false;
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
		// string macGw;
		// for (int i = 0; i < 5; i++)
		// {
		// 	macGw = mac.erase(mac.find(':'), 1);
		// }
		respValue["CMD"] = "HC_RESPONSE";
		respValue["IP"] = Wifi::GetIP();
		respValue["HOSTNAME"] = "RD_HC_" + mac.substr(mac.size() - 4, 4);
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
#ifdef CONFIG_USE_OLD_APP
	Json::Value wifiList;
	Json::Value wifi;
	Json::Value wifiResp;
	gateway->StopUdpBroadcast();
	Wifi::ScanWifi(wifiList);
	if (wifiList.isArray())
	{
		for (Json::ArrayIndex i = 0; i < wifiList.size(); i++)
		{
			wifi = wifiList[i];
			wifiResp["CMD"] = "HC_RESPONE";
			wifiResp["SSID"] = wifi["SSID"];
			wifiResp["QUALITY"] = 55;
			wifiResp["MAC"] = wifi["MAC"];
			wifiResp["ENCRYPTION"] = wifi["ENCRYPTION"];
			respValue.append(wifiResp);
		}
	}
	return 10; // respValue as an array
#else
	string rqi = "";
	if (reqValue.isMember("REQUEST_ID") && reqValue["REQUEST_ID"].isString())
	{
		rqi = reqValue["REQUEST_ID"].asString();
	}
	if (reqValue.isMember("FROM") && reqValue.isMember("TO"))
	{
		Json::Value from;
		Json::Value to;
		from = reqValue["FROM"];
		to = reqValue["TO"];
		if (from.isMember("TYPE") && from["TYPE"].isInt() && to.isMember("TYPE") && to["TYPE"].isInt())
		{
			if (from["TYPE"].asInt() == 0 && to["TYPE"].asInt() == 2)
			{
				Json::Value fromRsp;
				Json::Value toRsp;
				Json::Value dataRsp;
				StopUdpBroadcast();
				respValue["CMD"] = "HC_SCAN_WIFI_RESPONSE";
				respValue["REQUEST_ID"] = rqi;
				respValue["TIME"] = Util::GetCurrentTimeStr();
				respValue["CONNECTION_TYPE"] = 0;
				fromRsp["TYPE"] = 2;
				respValue["FROM"] = fromRsp;
				toRsp["TYPE"] = 0;
				respValue["TO"] = toRsp;
				Wifi::ScanWifi(dataRsp);
				respValue["DATA"] = dataRsp;
				return 0;
			}
			else
			{
				LOGW("OnUdpHcScanWifi payload: %s error direction", reqValue.toString().c_str());
			}
		}
		else
		{
			LOGW("OnUdpHcScanWifi payload: %s error", reqValue.toString().c_str());
		}
	}
	else
	{
		LOGW("OnUdpHcScanWifi payload: %s error", reqValue.toString().c_str());
	}

	return -1;
#endif
}

int Gateway::GatewayConnectToCloudNotice()
{
	LOGD("OnGatewayConnectToCloudNotice");
	Json::Value respValue;
	Json::Value from;
	Json::Value to;
	Json::Value data;

	respValue["CMD"] = "HC_UPDATE";
	respValue["REQUEST_ID"] = Util::genRandRQI(16);
	respValue["TIME"] = Util::GetCurrentTimeStr();
	respValue["CONNECTION_TYPE"] = 3;

	from["TYPE"] = 2;
	from["DORMITORY_ID"] = dormitoryId;
	from["IP"] = Wifi::GetIP();
	from["MAC"] = mac;
	from["VERSION"] = STR(VERSION);
	respValue["FROM"] = from;

	to["TYPE"] = 1;
	to["ENVIRONMENT"] = "test";
	respValue["TO"] = to;

	// respValue["DATA"] = data;

	return CloudPublish("HC.CONTROL.RESPONSE", respValue.toString());
}

int Gateway::OnUdpHcSetup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpHcSetup");
	string rqi = "";
	if (reqValue.isMember("REQUEST_ID") && reqValue["REQUEST_ID"].isString())
	{
		rqi = reqValue["REQUEST_ID"].asString();
	}
	if (reqValue.isMember("FROM") && reqValue.isMember("TO") && reqValue.isMember("DATA"))
	{
		Json::Value from = reqValue["FROM"];
		Json::Value to = reqValue["TO"];
		Json::Value data = reqValue["DATA"];

		string timeValue = Util::GetCurrentTimeStr();
		respValue["CMD"] = "SETUP_HC_RESPONSE";
		respValue["REQUEST_ID"] = rqi;
		respValue["TIME"] = timeValue;
		respValue["CONNECTION_TYPE"] = 0;

		Json::Value toRsp;
		Json::Value fromRsp;
		Json::Value dataRsp;

		toRsp["TYPE"] = 0;
		if (from.isMember("OS") && from["OS"].isString())
		{
			toRsp["OS"] = from["OS"].asString();
		}
		if (from.isMember("OS_VERSION") && from["OS_VERSION"].isString())
		{
			toRsp["OS_VERSION"] = from["OS_VERSION"].asString();
		}
		if (from.isMember("APP_BUILD") && from["APP_BUILD"].isString())
		{
			toRsp["APP_BUILD"] = from["APP_BUILD"].asString();
		}
		if (from.isMember("APP_VERSION") && from["APP_VERSION"].isString())
		{
			toRsp["APP_VERSION"] = from["APP_VERSION"].asString();
		}
		respValue["TO"] = toRsp;

		fromRsp["TYPE"] = 2;
		fromRsp["MAC"] = mac;
		fromRsp["VERSION"] = STR(VERSION);

		if (from.isMember("TYPE") && from["TYPE"].isInt() && to.isMember("TYPE") && to["TYPE"].isInt())
		{
			if ((from["TYPE"].asInt() == 0) && to["TYPE"].asInt())
			{
				if (data.isMember("DORMITORY_ID") && data["DORMITORY_ID"].isString())
				{
					dormitoryId = data["DORMITORY_ID"].asString();
					database->GatewayUpdateDormitory(gateway, dormitoryId);
#ifndef ESP_PLATFORM
					if (Wifi::GetIP().compare("10.10.10.1") != 0)
					{
						LOGI("Hc have IP: %s", Wifi::GetIP().c_str());
						dataRsp["STATUS"] = "SUCCESS";
						respValue["DATA"] = dataRsp;
						fromRsp["IP"] = Wifi::GetIP();
						fromRsp["DORMITORY_ID"] = dormitoryId;
						respValue["FROM"] = fromRsp;
						GatewayConnectToCloudNotice();
						return 0;
					}
					else
					{
#endif
						if (data.isMember("WIFI"))
						{
							Json::Value wifi = data["WIFI"];
							if (wifi.isMember("SSID") && wifi["SSID"].isString() &&
									wifi.isMember("PASSWORD") && wifi["PASSWORD"].isString() &&
									wifi.isMember("ENCRYPTION") && wifi["ENCRYPTION"].isString())
							{
								string ssid = wifi["SSID"].asString();
								string password = wifi["PASSWORD"].asString();
								string encryption = wifi["ENCRYPTION"].asString();
								LOGD("ssid: %s, password: %s, encryption: %s", ssid.c_str(), password.c_str(), encryption.c_str());

								if (Wifi::ConnectToWifi(ssid, password, encryption) == 0)
								{
									dataRsp["STATUS"] = "SUCCESS";
									respValue["DATA"] = dataRsp;
									fromRsp["IP"] = Wifi::GetIP();
									fromRsp["DORMITORY_ID"] = dormitoryId;
									respValue["FROM"] = fromRsp;
								}
								else
								{
									dataRsp["STATUS"] = "FAILED";
									respValue["DATA"] = dataRsp;
									fromRsp["IP"] = Wifi::GetIP();
									fromRsp["DORMITORY_ID"] = dormitoryId;
									respValue["FROM"] = fromRsp;
								}
								GatewayConnectToCloudNotice();
								return 0;
							}
							else
							{
								LOGW("OnUdpHcSetup don't have wifi data");
							}
						}
#ifndef ESP_PLATFORM
						else
						{
							LOGW("OnUdpHcSetup don't have wifi object");
						}
					}
#endif
				}
				else
				{
					LOGW("OnUdpHcSetup don't have dormitory");
				}
			}
			else
			{
				LOGW("OnUdpHcSetup error data from - to");
			}
		}
		else
		{
			LOGW("OnUdpHcSetup don't have from or to");
		}
	}
	else
	{
		LOGW("OnUdpHcSetup payload: %s error", reqValue.toString().c_str());
	}
	return -1;
}

int Gateway::OnUdpHcConnectCloud(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpHcConnectCloud");
	return 1;
}

int Gateway::OnRPCHcConnectCloud(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpHcConnectCloud");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("DORMITORY_ID") && data["DORMITORY_ID"].isString() && data.isMember("REFRESH_TOKEN") && data["REFRESH_TOKEN"].isString())
		{
			string dormitoryId = data["DORMITORY_ID"].asString();
			string refreshToken = data["REFRESH_TOKEN"].asString();
			gateway->setDormitory(dormitoryId);
			gateway->setRefreshToken(refreshToken);
			database->GatewayUpdateDormitory(gateway, dormitoryId);
			database->GatewayUpdateRefreshToken(gateway, refreshToken);
		}
	}
	else
	{
		LOGW("OnUdpHcConnectCloud %s error", reqValue.toString().c_str());
	}
	return 1;
}

int Gateway::OnRPCHcBackup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCHcBackup");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		HTTPRequest *httpRequest = new HTTPRequest("POST", string(BASE_URL_DEV) + string(RENEW_TOKEN), "");
		string rs = httpRequest->UploadFile(gateway->refresh_token, gateway->dormitoryId, DB_NAME);
	}
	else
	{
		LOGW("OnRPCHcBackup %s error", reqValue.toString().c_str());
	}
	return 1;
}

int Gateway::OnRPCBleStartScan(Json::Value &reqValue, Json::Value &respValue)
{
	scanDeviceList.clear();
	bleProtocol->isAdding = true;
	bleProtocol->isProvisioning = true;
	if (bleProtocol->StartScan())
	{
		bleProtocol->StopScan();
	}
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCBleStopScan(Json::Value &reqValue, Json::Value &respValue)
{
	bleProtocol->StopScan();
	bleProtocol->isAdding = false;
	bleProtocol->isProvisioning = false;
	respValue = reqValue;
	return 0;
}

int Gateway::OnRPCBleReset(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("Reset ble");
	bleProtocol->ResetFactory();
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCResetFactory(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("Reset ble");
	ResetFactory();
	respValue["CMD"] = "RESET_HC";
	Json::Value data;
	data["STATUS"] = "SUCCESS";
	respValue["DATA"] = data;
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
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			string deviceId = dataValue[i].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				bleProtocol->ResetDev(device->GetAddr());
				database->DeviceDel(device->GetMac());
				LOGD("remove deviceId: %s", deviceId.c_str());
			}
			else
			{
				LOGD("deviceId %s dose not exist", deviceId.c_str());
			}
		}
	}
	else
	{
		LOGW("Format error");
	}
	return 0;
}

int Gateway::OnRPCAddRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddRule");
	respValue["CMD"] = "CREATE_EVENT_TRIGGER";
	Json::Value dataJsonRsp = Json::objectValue;
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string eventId = dataValue["EVENT_TRIGGER_ID"].asString();
			dataJsonRsp["EVENT_TRIGGER_ID"] = eventId;
			Rule *rule = AddRule(dataValue, true, true);
			if (rule)
			{
				dataJsonRsp["STATUS"] = "SUCCESS";
			}
			else
			{
				dataJsonRsp["STATUS"] = "FAILED";
			}
		}
	}
	respValue["DATA"] = dataJsonRsp;
	return 0;
}

int Gateway::OnRPCEditRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCEditRule");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "EDIT_EVENT_TRIGGER";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string eventId = dataValue["EVENT_TRIGGER_ID"].asString();
			dataJsonRsp["EVENT_TRIGGER_ID"] = eventId;
			Rule *rule = getRuleById(eventId);
			if (rule)
			{
				rule->DelAllRuleInput();
				rule->DelAllRuleOutput();
				rule = AddRule(dataValue, true, true);
				if (rule)
				{
					dataJsonRsp["STATUS"] = "SUCCESS";
				}
				else
				{
					dataJsonRsp["STATUS"] = "FAILED";
				}
			}
			else
			{
				LOGW("rule %s does not exsit", eventId.c_str());
			}
		}
		respValue["DATA"] = dataJsonRsp;
		return 0;
	}
	return -1;
}

int Gateway::OnRPCSwitchStatusEvent(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCSwitchStatusEvent");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "EVENT_TRIGGER_STATUS";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("STATUS_ID") && dataValue["STATUS_ID"].isInt() && dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string ruleId = dataValue["EVENT_TRIGGER_ID"].asString();
			int status = dataValue["STATUS_ID"].asInt();
			dataJsonRsp["EVENT_TRIGGER_ID"] = ruleId;
			dataJsonRsp["STATUS_ID"] = status;
			Rule *rule = getRuleById(ruleId);
			if (rule)
			{
				rule->isEnable = (status) ? true : false;
				database->RuleUpdateStatus(ruleId, status);
			}
			else
			{
				LOGW("Switch rule %s does not exsit", ruleId.c_str());
			}
		}
		else
		{
			LOGW("OnRPCSwitchStatusEvent msg enough info");
		}
		respValue["DATA"] = dataJsonRsp;
		return 0;
	}
	return -1;
}

int Gateway::OnRPCDeleteRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDeleteRule");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "DELETE_EVENT_TRIGGER";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string ruleId = dataValue["EVENT_TRIGGER_ID"].asString();
			dataJsonRsp["EVENT_TRIGGER_ID"] = ruleId;
			LOGI("Delete Rule id: %s", ruleId.c_str());
			delete ruleList[ruleId];
			ruleList.erase(ruleList.find(ruleId));
			database->RuleDel(ruleId);
		}
		respValue["DATA"] = dataJsonRsp;
		return 0;
	}
	return -1;
}

int Gateway::OnRPCCreateHCL(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		respValue["CMD"] = "EVENT_TRIGGER";
		Json::Value dataJsonRsp = Json::objectValue;
		if (data.isMember("EVENT_TRIGGER_ID") && data["EVENT_TRIGGER_ID"].isString() &&
				data.isMember("EACH_DAY") && data["EACH_DAY"].isArray() &&
				data.isMember("GROUP_ID") && data["GROUP_ID"].isString() &&
				data.isMember("STATUS") && data["STATUS"].isInt() &&
				data.isMember("STATES") && data["STATES"].isArray())
		{
			string evevtId = data["EVENT_TRIGGER_ID"].asString();
			dataJsonRsp["EVENT_TRIGGER_ID"] = evevtId;
			string groupId = data["GROUP_ID"].asString();
			Json::Value eachDay = data["EVENT_TRIGGER_ID"];
			int status = data["STATUS"].asInt();
			Json::Value states = data["STATES"];
			if (states.isMember("TIME") && states["TIME"].isString() && states.isMember("PROPERTIES") && states["PROPERTIES"].isArray())
			{
				string time = states["TIME"].asString();
				Json::Value properties = states["PROPERTIES"];
				Json::Value dataAddRule;
				dataAddRule["EVENT_TRIGGER_ID"] = evevtId;
				dataAddRule["START_AT"] = time;
				dataAddRule["EACH_DAY"] = eachDay;
				dataAddRule["LOGICAL_OPERATOR_ID"] = 0;
				dataAddRule["STATUS"] = status;
				Json::Value outputGroup;
				outputGroup["GROUP_ID"] = groupId;
				outputGroup["PROPERTIES"] = properties;
				dataAddRule["OUTPUT_GROUPS"] = outputGroup;
				Rule *rule = AddRule(dataAddRule, true, true);
				if (rule)
				{
					dataJsonRsp["STATUS"] = "SUCCESS";
				}
				else
				{
					dataJsonRsp["STATUS"] = "FAILED";
				}
			}
		}
		else
		{
			LOGW("Error adding rule");
		}
		respValue["DATA"] = dataJsonRsp;
		return 0;
	}
	return -1;
}

int Gateway::OnRPCEditHCL(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		respValue["CMD"] = "EDIT_EVENT_TRIGGER";
		Json::Value dataJsonRsp = Json::objectValue;
		if (data.isMember("EVENT_TRIGGER_ID") && data["EVENT_TRIGGER_ID"].isString() &&
				data.isMember("EACH_DAY") && data["EACH_DAY"].isArray() &&
				data.isMember("GROUP_ID") && data["GROUP_ID"].isString() &&
				data.isMember("STATUS") && data["STATUS"].isInt() &&
				data.isMember("STATES") && data["STATES"].isArray())
		{
			string evevtId = data["EVENT_TRIGGER_ID"].asString();
			dataJsonRsp["EVENT_TRIGGER_ID"] = evevtId;
			string groupId = data["GROUP_ID"].asString();
			Json::Value eachDay = data["EVENT_TRIGGER_ID"];
			int status = data["STATUS"].asInt();
			Json::Value states = data["STATES"];
			if (states.isMember("TIME") && states["TIME"].isString() && states.isMember("PROPERTIES") && states["PROPERTIES"].isArray())
			{
				string time = states["TIME"].asString();
				Json::Value properties = states["PROPERTIES"];
				Json::Value dataAddRule;
				dataAddRule["EVENT_TRIGGER_ID"] = evevtId;
				dataAddRule["START_AT"] = time;
				dataAddRule["EACH_DAY"] = eachDay;
				dataAddRule["LOGICAL_OPERATOR_ID"] = 0;
				dataAddRule["STATUS"] = status;
				Json::Value outputGroup;
				outputGroup["GROUP_ID"] = groupId;
				outputGroup["PROPERTIES"] = properties;
				dataAddRule["OUTPUT_GROUPS"] = outputGroup;
				Rule *rule = getRuleById(evevtId);
				if (rule)
				{
					rule->DelAllRuleInput();
					rule->DelAllRuleOutput();
					rule = AddRule(dataAddRule, true, true);
					dataJsonRsp["STATUS"] = "SUCCESS";
				}
				else
				{
					dataJsonRsp["STATUS"] = "FAILED";
				}
			}
		}
		else
		{
			LOGW("Error adding rule");
		}
		respValue["DATA"] = dataJsonRsp;
		return 0;
	}
	return -1;
}

int Gateway::OnRPCAddSceneBle(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddSceneBle");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "CREATE_SCENE";
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["FAILED"] = Json::arrayValue;

		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString() &&
				dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
		{
			string sceneId = dataValue["SCENE_ID"].asString();
			dataJsonRsp["SCENE_ID"] = sceneId;
			int temp_sceneUnicastId = 1;
			for (auto &x : sceneBleList)
			{
				if (x.first >= temp_sceneUnicastId)
				{
					temp_sceneUnicastId = x.first + 1;
				}
			}
			SceneBle *scene = new SceneBle(sceneId, temp_sceneUnicastId, sceneId);
			if (scene)
			{
				scene = AddNewSceneBle(scene, true, true);
				if (scene)
				{
					Json::Value groupList = dataValue["DEVICES"];
					for (Json::ArrayIndex i = 0; i < groupList.size(); i++)
					{
						Json::Value devInfo = groupList[i];
						if (devInfo.isMember("IDS") && devInfo["IDS"].isArray() && devInfo.isMember("PROPERTIES") && devInfo["PROPERTIES"].isArray())
						{
							Json::Value deviceList = devInfo["IDS"];
							Json::Value deviceProperties = devInfo["PROPERTIES"];

							int modeRgb = 0;
							for (Json::ArrayIndex j = 0; j < deviceProperties.size(); j++)
							{
								Json::Value property = deviceProperties[j];
								if (property.isMember("ID") && property["ID"].isInt() && property.isMember("VALUE") && property["VALUE"].isInt())
								{
									if (property["ID"].asInt() == 23)
									{
										modeRgb = property["VALUE"].asInt();
									}
								}
							}
							for (Json::ArrayIndex j = 0; j < deviceList.size(); j++)
							{
								string devcieId = deviceList[j].asString();
								Device *device = getDeviceFromId(devcieId);
								if (device)
								{
									if (scene->AddDevice(device, deviceProperties, modeRgb, false))
									{
										database->DeviceInSceneBleAdd(scene, device, deviceProperties);
										dataJsonRsp["SUCCESS"].append(device->GetId());
									}
									else
									{
										dataJsonRsp["FAILED"].append(device->GetId());
									}
								}
							}
						}
					}
				}
			}
			else
			{
				LOGW("Create new scene error");
			}
		}
		respValue["DATA"] = dataJsonRsp;
	}
	return 0;
}

int Gateway::OnRPCEditSceneBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString() && dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
		{
			respValue["CMD"] = "EDIT_SCENE";
			Json::Value dataJsonRsp = Json::objectValue;
			dataJsonRsp["FAILED"] = Json::arrayValue;
			string sceneId = dataValue["SCENE_ID"].asString();
			dataJsonRsp["SCENE_ID"] = sceneId;
			SceneBle *scene = getSceneBleFromId(sceneId);
			if (scene)
			{
				Json::Value groupList = dataValue["DEVICES"];
				for (Json::ArrayIndex i = 0; i < groupList.size(); i++)
				{
					Json::Value devInfo = groupList[i];
					if (devInfo.isMember("IDS") && devInfo["IDS"].isArray() && devInfo.isMember("PROPERTIES") && devInfo["PROPERTIES"].isArray())
					{
						Json::Value deviceList = devInfo["IDS"];
						Json::Value deviceProperties = devInfo["PROPERTIES"];

						int modeRgb = 0;
						for (Json::ArrayIndex m = 0; m < deviceProperties.size(); m++)
						{
							Json::Value property = deviceProperties[m];
							if (property.isMember("ID") && property["ID"].isInt() && property.isMember("VALUE") && property["VALUE"].isInt())
							{
								if (property["ID"].asInt() == 23)
								{
									modeRgb = property["VALUE"].asInt();
								}
							}
						}
						for (Json::ArrayIndex j = 0; j < deviceList.size(); j++)
						{
							string deviceId = deviceList[j].asString();
							Device *device = getDeviceFromId(deviceId);
							if (device)
							{
								if (scene->AddDevice(device, deviceProperties, modeRgb, false))
								{
									database->DeviceInSceneBleAdd(scene, device, deviceProperties);
									dataJsonRsp["SUCCESS"].append(deviceId);
								}
								else
								{
									dataJsonRsp["FAILED"].append(deviceId);
								}
							}
							else
							{
								LOGW("Device %s does not exsit", deviceId.c_str())
							}
						}
					}
				}
			}
			else
			{
				LOGW("Scene %s does not exsit", sceneId.c_str());
			}
			respValue["DATA"] = dataJsonRsp;
		}
	}
	return 0;
}

int Gateway::OnRPCDeleteSceneBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "DELETE_SCENE";
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["FAILED"] = Json::arrayValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString())
		{
			string sceneId = dataValue["SCENE_ID"].asString();
			dataJsonRsp["SCENE_ID"] = sceneId;
			SceneBle *scene = getSceneBleFromId(sceneId);

			if (scene)
			{
				vector<Device *> listDevInScene;
				int numDevInScene = scene->deviceList.size();
				for (int j = 0; j < numDevInScene; j++)
				{
					listDevInScene.push_back(scene->deviceList[j]->device);
				}
				for (int i = 0; i < numDevInScene; i++)
				{
					if (scene->DelDevice(listDevInScene[i]))
					{
						dataJsonRsp["SUCCESS"].append(listDevInScene[i]->GetId());
						database->DeviceInSceneBleDel(scene, listDevInScene[i], 0);
					}
					else
					{
						dataJsonRsp["FAILED"].append(scene->deviceList[i]->device->GetId());
					}
				}
				delete sceneBleList[scene->GetId()];
			}
			else
			{
				LOGW("Scene %s does not exsit", sceneId.c_str());
			}
		}
		respValue["DATA"] = dataJsonRsp;
	}
	return 0;
}

int Gateway::OnRPCCreateRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCCreateRoom %s", reqValue.toString().c_str());
	bool isRoom = false;
	string roomId = "";
	int roomUnicast = 0;
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("GROUPS") && data["GROUPS"].isArray() && data.isMember("SCENES") && data["SCENES"].isArray())
		{
			Json::Value groups = data["GROUPS"];
			respValue["CMD"] = "CREATE_ROOM";
			Json::Value jsonDataRsp;
			Json::Value jsonGroupsRsp;
			for (Json::ArrayIndex i = 0; i < groups.size(); i++)
			{
				Json::Value group = groups[i];
				Json::Value jsonGroupRsp;
				if (group.isMember("GROUP_ID") && group["GROUP_ID"].isString() && group.isMember("NAME") && group["NAME"].isString())
				{
					string groupId = group["GROUP_ID"].asString();
					string groupName = group["NAME"].asString();

					jsonGroupRsp["GROUP_ID"] = groupId;
					jsonGroupRsp["FAILED"] = Json::arrayValue;
					int temp_groupUnicastId = 1;
					for (auto &x : groupList)
					{
						if (x.first >= temp_groupUnicastId)
						{
							temp_groupUnicastId = x.first + 1;
						}
					}
					if (!isRoom)
					{
						isRoom = true;
						roomId = groupId;
						roomUnicast = temp_groupUnicastId;
					}
					Group *newGroup = new Group(groupId, temp_groupUnicastId, groupName);
					if (newGroup)
					{
						jsonGroupRsp["GROUP_UNICAST_ID"] = temp_groupUnicastId + 49152;
						if (AddNewGroup(newGroup, true, true))
						{
							Json::Value data;
							if (group.isMember("DEVICES") && group["DEVICES"].isArray())
							{
								Json::Value devices = group["DEVICES"];
								for (Json::ArrayIndex j = 0; j < devices.size(); j++)
								{
									string devcieId = devices[j].asString();
									Device *device = getDeviceFromId(devcieId);
									if (device)
									{
										int tempDeviceAddr = device->GetAddr();
										if (newGroup->AddDevice(device, tempDeviceAddr, true))
										{
											database->DeviceInGroupAdd(newGroup, device, tempDeviceAddr);
											jsonGroupRsp["SUCCESS"].append(device->GetId());
										}
										else
										{
											jsonGroupRsp["FAILED"].append(device->GetId());
										}
									}
								}
							}
						}
						else
						{
							delete newGroup;
						}
						jsonDataRsp["GROUPS"].append(jsonGroupRsp);
						if (jsonGroupRsp.isMember("SUCCESS"))
							jsonGroupRsp["SUCCESS"].clear();
						if (jsonGroupRsp.isMember("FAILED"))
							jsonGroupRsp["FAILED"].clear();
						jsonGroupRsp["FAILED"] = Json::arrayValue;
					}
				}
			}

			Json::Value scenes = data["SCENES"];
			Json::Value jsonScenesRsp;
			Json::Value jsonSceneRsp;
			for (int i = 0; i < scenes.size(); i++)
			{
				Json::Value scene = scenes[i];
				if (scene.isMember("SCENE_ID") && scene["SCENE_ID"].isString() && scene.isMember("SCENE_NAME") && scene["SCENE_NAME"].isString() && scene.isMember("GROUPS") && scene["GROUPS"].isArray())
				{
					string sceneId = scene["SCENE_ID"].asString();
					string sceneName = scene["SCENE_NAME"].asString();
					jsonSceneRsp["SCENE_ID"] = sceneId;
					jsonSceneRsp["FAILED"] = Json::arrayValue;
					int temp_sceneUnicastId = 1;
					for (auto &x : sceneBleList)
					{
						if (x.first >= temp_sceneUnicastId)
						{
							temp_sceneUnicastId = x.first + 1;
						}
					}
					SceneBle *sceneInRoom = new SceneBle(sceneId, temp_sceneUnicastId, sceneName);
					if (sceneInRoom)
					{
						jsonSceneRsp["SCENE_UNICAST_ID"] = temp_sceneUnicastId;
						sceneInRoom = AddNewSceneBle(sceneInRoom, true, true);
					}

					Json::Value groupsOfScene = scene["GROUPS"];
					for (Json::ArrayIndex j = 0; j < groupsOfScene.size(); j++)
					{
						Json::Value groupOfScene = groupsOfScene[j];
						if (groupOfScene.isMember("GROUP_ID") && groupOfScene["GROUP_ID"].isString() && groupOfScene.isMember("PROPERTIES") && groupOfScene["PROPERTIES"].isArray())
						{
							string idGroup = groupOfScene["GROUP_ID"].asString();
							Json::Value properties = groupOfScene["PROPERTIES"];
							Group *groupInScene = gateway->getGroupFromId(idGroup);
							if (groupInScene)
							{
								groupInScene->Do(properties);
							}
							else
							{
								LOGW("Group %s does not exsit", idGroup.c_str());
							}

							int modeRGB = 0;
							for (Json::ArrayIndex k = 0; k < properties.size(); k++)
							{
								Json::Value property = properties[k];
								if (property.isMember("ID") && property["ID"].isInt() && property.isMember("VALUE") && property["VALUE"].isInt())
								{
									if (property["ID"].asInt() == 23)
									{
										modeRGB = property["VALUE"].asInt();
									}
								}
							}

							if (sceneInRoom)
							{
								for (auto n = 0; n < groupInScene->deviceList.size(); n++)
								{
									string devcieId = groupInScene->deviceList[n]->device->GetId();
									Device *deviceInScene = getDeviceFromId(devcieId);
									if (deviceInScene)
									{
										int tempDeviceAddr = deviceInScene->GetAddr();
										if (sceneInRoom->AddDevice(deviceInScene, properties, modeRGB, false))
										{
											database->DeviceInSceneBleAdd(sceneInRoom, deviceInScene, properties);
											jsonSceneRsp["SUCCESS"].append(deviceInScene->GetId());
										}
										else
										{
											jsonSceneRsp["FAILED"].append(deviceInScene->GetId());
										}
									}
								}
							}
						}
					}
					jsonDataRsp["SCENES"].append(jsonSceneRsp);
					if (jsonSceneRsp.isMember("SUCCESS"))
						jsonSceneRsp.removeMember("SUCCESS");
					if (jsonSceneRsp.isMember("FAILED"))
						jsonSceneRsp.removeMember("FAILED");
					jsonSceneRsp["FAILED"] = Json::arrayValue;
				}
			}
			respValue["DATA"] = jsonDataRsp;
			if (roomId != "")
			{
				Room *room = getRoomFromId(roomId);
				if (!room)
				{
					room = new Room(roomId, roomUnicast);
					room = gateway->AddNewRoom(room);
					database->RoomAdd(room);
				}
				room->DataConfigAdd(respValue.toString());
				string ruleStr = respValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->DataRoomAdd(roomId, respValue.toString());
			}
		}
		else
		{
			LOGW("OnRPCCreateRoom error: %s", respValue.toString().c_str());
		}
	}
	return 0;
}

int Gateway::OnRPCAddDevToRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddDevToRoom %s", reqValue.toString().c_str());
	vector<string> listDevAddGroup;
	map<string, vector<string>> listGroupDevAddRoom;

	bool isRoom = false;
	string roomId = "";
	int roomUnicast = 0;
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("GROUPS") && data["GROUPS"].isArray() && data.isMember("SCENES") && data["SCENES"].isArray())
		{
			respValue["CMD"] = "ADD_DEVICE_TO_ROOM";
			Json::Value groupJsonRsp;
			Json::Value sceneJsonRsp;
			Json::Value dataJsonRsp = Json::objectValue;
			groupJsonRsp["FAILED"] = Json::arrayValue;
			sceneJsonRsp["FAILED"] = Json::arrayValue;

			Json::Value groupsAddDev = data["GROUPS"];
			for (Json::ArrayIndex i = 0; i < groupsAddDev.size(); i++)
			{
				Json::Value groupAddDev = groupsAddDev[i];
				if (groupAddDev.isMember("GROUP_ID") && groupAddDev["GROUP_ID"].isString() && groupAddDev.isMember("DEVICES") && groupAddDev["DEVICES"].isArray())
				{
					string groupId = groupAddDev["GROUP_ID"].asString();
					groupJsonRsp["GROUP_ID"] = groupId;
					Json::Value devicesInGroupAddRoom = groupAddDev["DEVICES"];
					Group *groupOfGw = gateway->getGroupFromId(groupId);
					if (groupOfGw)
					{
						if (!isRoom)
						{
							isRoom = true;
							roomId = groupId;
							roomUnicast = groupOfGw->GetId();
						}
						for (Json::ArrayIndex j = 0; j < devicesInGroupAddRoom.size(); j++)
						{
							string deviceIdGroup = devicesInGroupAddRoom[j].asString();
							Device *deviceAddtoRoom = gateway->getDeviceFromId(deviceIdGroup);
							if (deviceAddtoRoom)
							{
								int devAddr = deviceAddtoRoom->GetAddr();
								listDevAddGroup.push_back(deviceIdGroup);
								if (groupOfGw->AddDevice(deviceAddtoRoom, devAddr, true))
								{
									database->DeviceInGroupAdd(groupOfGw, deviceAddtoRoom, devAddr);
									groupJsonRsp["SUCCESS"].append(deviceIdGroup);
								}
								else
								{
									groupJsonRsp["FAILED"].append(deviceIdGroup);
								}
							}
							else
							{
								LOGW("Device %s does not exsit", deviceIdGroup.c_str());
							}
						}

						listGroupDevAddRoom[groupId] = listDevAddGroup;
						listDevAddGroup.clear();
						dataJsonRsp["GROUPS"].append(groupJsonRsp);
						if (groupJsonRsp.isMember("SUCCESS"))
							groupJsonRsp.removeMember("SUCCESS");
						if (groupJsonRsp.isMember("FAILED"))
							groupJsonRsp.removeMember("FAILED");
						groupJsonRsp["FAILED"] = Json::arrayValue;
					}
					else
					{
						string nameGroup;
						if (groupAddDev.isMember("NAME") && groupAddDev["NAME"].isString())
						{
							nameGroup = groupAddDev["NAME"].asString();
						}
						int temp_groupUnicastId = 1;
						for (auto &x : groupList)
						{
							if (x.first >= temp_groupUnicastId)
							{
								temp_groupUnicastId = x.first + 1;
							}
						}
						Group *newGroup = new Group(groupId, temp_groupUnicastId, nameGroup);
						if (newGroup)
						{
							groupJsonRsp["GROUP_UNICAST_ID"] = temp_groupUnicastId + 49152;
							if (AddNewGroup(newGroup, true, true))
							{
								Json::Value data;

								Json::Value devices = groupAddDev["DEVICES"];
								for (Json::ArrayIndex j = 0; j < devices.size(); j++)
								{
									string deviceIdScene = devices[j].asString();
									Device *device = getDeviceFromId(deviceIdScene);
									if (device)
									{
										int tempDeviceAddr = device->GetAddr();
										listDevAddGroup.push_back(deviceIdScene);
										if (newGroup->AddDevice(device, tempDeviceAddr, true))
										{
											database->DeviceInGroupAdd(newGroup, device, tempDeviceAddr);
											groupJsonRsp["SUCCESS"].append(deviceIdScene);
										}
										else
										{
											groupJsonRsp["FAILED"].append(deviceIdScene);
										}
									}
								}
								listGroupDevAddRoom[groupId] = listDevAddGroup;
								listDevAddGroup.clear();
							}
							else
							{
								delete newGroup;
							}
							dataJsonRsp["GROUPS"].append(groupJsonRsp);
							if (groupJsonRsp.isMember("SUCCESS"))
								groupJsonRsp["SUCCESS"].clear();
							if (groupJsonRsp.isMember("FAILED"))
								groupJsonRsp["FAILED"].clear();
							groupJsonRsp["FAILED"] = Json::arrayValue;
						}
					}
				}
			}

			Json::Value scenesAddDev = data["SCENES"];
			for (Json::ArrayIndex n = 0; n < scenesAddDev.size(); n++)
			{
				Json::Value sceneAddDev = scenesAddDev[n];
				if (sceneAddDev.isMember("SCENE_ID") && sceneAddDev["SCENE_ID"].isString() && sceneAddDev.isMember("GROUPS") && sceneAddDev["GROUPS"].isArray())
				{
					string sceneId = sceneAddDev["SCENE_ID"].asString();
					Json::Value infoDevsAdd = sceneAddDev["GROUPS"];
					sceneJsonRsp["SCENE_ID"] = sceneId;
					SceneBle *sceneOfGw = gateway->getSceneBleFromId(sceneId);
					if (sceneOfGw)
					{
						for (Json::ArrayIndex l = 0; l < infoDevsAdd.size(); l++)
						{
							Json::Value infoDevAdd = infoDevsAdd[l];
							if (infoDevAdd.isMember("GROUP_ID") && infoDevAdd["GROUP_ID"].isString() && infoDevAdd.isMember("PROPERTIES") && infoDevAdd["PROPERTIES"].isArray())
							{
								string groupIdInScene = infoDevAdd["GROUP_ID"].asString();
								Json::Value properties = infoDevAdd["PROPERTIES"];
								Group *groupOfGw = gateway->getGroupFromId(groupIdInScene);
								if (groupOfGw)
								{
									groupOfGw->Do(properties);
								}
								else
								{
									LOGW("Group %s does not exsit", groupIdInScene.c_str());
								}

								int mode = 0;
								for (Json::ArrayIndex m = 0; m < properties.size(); m++)
								{
									Json::Value property = properties[m];
									if (property.isMember("ID") && property["ID"].isInt() && property.isMember("VALUE") && property["VALUE"].isInt())
									{
										if (property["ID"].asInt() == 23)
										{
											mode = property["VALUE"].asInt();
										}
									}
								}
								for (int g = 0; g < listGroupDevAddRoom[groupIdInScene].size(); g++)
								{
									string deviceIdInScene = listGroupDevAddRoom[groupIdInScene][g];
									Device *deviceInScene = getDeviceFromId(deviceIdInScene);
									if (deviceInScene)
									{
										int adrDev = deviceInScene->GetAddr();
										if (sceneOfGw->AddDevice(deviceInScene, properties, mode, false))
										{
											database->DeviceInSceneBleAdd(sceneOfGw, deviceInScene, properties);
											sceneJsonRsp["SUCCESS"].append(deviceIdInScene);
										}
										else
										{
											sceneJsonRsp["FAILED"].append(deviceIdInScene);
										}
									}
								}
							}
						}
					}
					else
					{
						LOGW("Scene %s does not exsit", sceneId.c_str());
					}
					dataJsonRsp["SCENES"].append(sceneJsonRsp);
					if (sceneJsonRsp.isMember("SUCCESS"))
						sceneJsonRsp.removeMember("SUCCESS");
					if (sceneJsonRsp.isMember("FAILED"))
						sceneJsonRsp.removeMember("FAILED");
					sceneJsonRsp["FAILED"] = Json::arrayValue;
				}
			}
			respValue["DATA"] = dataJsonRsp;
			if (roomId != "" && roomUnicast != 0)
			{
				Room *room = getRoomFromId(roomId);
				if (!room)
				{
					room = new Room(roomId, roomUnicast);
					room = gateway->AddNewRoom(room);
					database->RoomAdd(room);
				}
				room->DataConfigAdd(respValue.toString());
				string ruleStr = respValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->DataRoomAdd(roomId, respValue.toString());
			}
		}
		else
		{
			LOGW("OnRPCAddDevToRoom error: %s", respValue.toString().c_str());
		}
	}
	return 0;
}

int Gateway::OnRPCRemoveDevFromRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCRemoveDevFromRoom: %s", reqValue.toString().c_str());
	bool isRoom = false;
	string roomId = "";
	int roomUnicast = 0;

	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("GROUPS") && data["GROUPS"].isArray() && data.isMember("SCENES") && data["SCENES"].isArray())
		{
			respValue["CMD"] = "REMOVE_DEVICE_FROM_ROOM";
			Json::Value groupsDelRoom = data["GROUPS"];
			Json::Value scenesDelRoom = data["SCENES"];
			Json::Value dataJsonRsp = Json::objectValue;
			Json::Value groupJsonRsp;
			Json::Value sceneJsonRsp;
			groupJsonRsp["FAILED"] = Json::arrayValue;
			sceneJsonRsp["FAILED"] = Json::arrayValue;

			for (Json::ArrayIndex i = 0; i < groupsDelRoom.size(); i++)
			{
				Json::Value groupDelRoom = groupsDelRoom[i];
				if (groupDelRoom.isMember("GROUP_ID") && groupDelRoom["GROUP_ID"].isString())
				{
					string groupId = groupDelRoom["GROUP_ID"].asString();
					groupJsonRsp["GROUP_ID"] = groupId;
					Group *groupOfGw = getGroupFromId(groupId);
					if (groupOfGw)
					{
						if (!isRoom)
						{
							isRoom = true;
							roomId = groupId;
							roomUnicast = groupOfGw->GetId();
						}
						if (groupDelRoom.isMember("DEVICES") && groupDelRoom["DEVICES"].isArray())
						{
							Json::Value devicesDelGroup = groupDelRoom["DEVICES"];
							for (Json::ArrayIndex j = 0; j < devicesDelGroup.size(); j++)
							{
								string deviceId = devicesDelGroup[j].asString();
								Device *deviceDelGroup = getDeviceFromId(deviceId);
								if (deviceDelGroup)
								{
									int adrDev = deviceDelGroup->GetAddr();
									if (groupOfGw->DelDevice(deviceDelGroup, adrDev))
									{
										database->DeviceInGroupDel(groupOfGw, deviceDelGroup, adrDev);
										groupJsonRsp["SUCCESS"].append(deviceId);
									}
									else
									{
										groupJsonRsp["FAILED"].append(deviceId);
									}
								}
								else
								{
									LOGW("Device %s does not exsit", deviceId.c_str());
								}
							}
						}
					}
					else
					{
						LOGW("Group %s does not exsit", groupId.c_str());
					}
				}
				dataJsonRsp["GROUPS"].append(groupJsonRsp);
				if (groupJsonRsp.isMember("SUCCESS"))
					groupJsonRsp.removeMember("SUCCESS");
				if (groupJsonRsp.isMember("FAILED"))
					groupJsonRsp.removeMember("FAILED");
				groupJsonRsp["FAILED"] = Json::arrayValue;
			}

			for (Json::ArrayIndex n = 0; n < scenesDelRoom.size(); n++)
			{
				Json::Value sceneDelRoom = scenesDelRoom[n];
				if (sceneDelRoom.isMember("SCENE_ID") && sceneDelRoom["SCENE_ID"].isString())
				{
					string sceneId = sceneDelRoom["SCENE_ID"].asString();
					sceneJsonRsp["SCENE_ID"] = sceneId;
					SceneBle *sceneOfGw = getSceneBleFromId(sceneId);
					if (sceneOfGw)
					{
						if (sceneDelRoom.isMember("DEVICES") && sceneDelRoom["DEVICES"].isArray())
						{
							Json::Value devicesDelScene = sceneDelRoom["DEVICES"];
							for (Json::ArrayIndex m = 0; m < devicesDelScene.size(); m++)
							{
								string deviceIdDelScene = devicesDelScene[m].asString();
								Device *deviceDelScene = getDeviceFromId(deviceIdDelScene);
								if (deviceDelScene)
								{
									if (sceneOfGw->DelDevice(deviceDelScene))
									{
										database->DeviceInSceneBleDel(sceneOfGw, deviceDelScene, 0);
										sceneJsonRsp["SUCCESS"].append(deviceIdDelScene);
									}
									else
									{
										sceneJsonRsp["FAILED"].append(deviceIdDelScene);
									}
								}
								else
								{
									LOGW("Device %s does not exsit", deviceIdDelScene.c_str());
								}
							}
						}
					}
					else
					{
						LOGW("Scene %s does not exsit", sceneId.c_str());
					}
				}
				dataJsonRsp["SCENES"].append(sceneJsonRsp);
				if (sceneJsonRsp.isMember("SUCCESS"))
					sceneJsonRsp.removeMember("SUCCESS");
				if (sceneJsonRsp.isMember("FAILED"))
					sceneJsonRsp.removeMember("FAILED");
				sceneJsonRsp["FAILED"] = Json::arrayValue;
			}
			respValue["DATA"] = dataJsonRsp;
			if (roomId != "" && roomUnicast != 0)
			{
				Room *room = getRoomFromId(roomId);
				if (!room)
				{
					room = new Room(roomId, roomUnicast);
					room = gateway->AddNewRoom(room);
					database->RoomAdd(room);
				}
				room->DataConfigAdd(respValue.toString());
				string ruleStr = respValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->DataRoomAdd(roomId, respValue.toString());
			}
		}
		else
		{
			LOGW("OnRPCRemoveDevFromRoom msg error");
		}
	}
	LOGE("Return");
	return 0;
}

int Gateway::OnRPCDeleteRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDeleteRoom: %s", reqValue.toString().c_str());
	bool isRoom = false;
	string roomId = "";
	int roomUnicast = 0;
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("GROUPS") && data["GROUPS"].isArray() && data.isMember("SCENES") && data["SCENES"].isArray())
		{
			respValue["CMD"] = "DELETE_ROOM";
			Json::Value groupsDelRoom = data["GROUPS"];
			Json::Value scenesDelRoom = data["SCENES"];
			Json::Value dataJsonRsp = Json::objectValue;
			Json::Value groupJsonRsp;
			Json::Value sceneJsonRsp;
			groupJsonRsp["FAILED"] = Json::arrayValue;
			sceneJsonRsp["FAILED"] = Json::arrayValue;

			bool hasDeviceDelGroupFailed;
			bool hasDeviceDelSceneFailed;

			for (Json::ArrayIndex i = 0; i < groupsDelRoom.size(); i++)
			{
				string groupId = groupsDelRoom[i].asString();
				groupJsonRsp["GROUP_ID"] = groupId;
				Group *groupOfGw = getGroupFromId(groupId);
				if (groupOfGw)
				{
					if (!isRoom)
					{
						isRoom = true;
						roomId = groupId;
						roomUnicast = groupOfGw->GetId();
					}
					hasDeviceDelGroupFailed = false;
					for (int j = 0; j < groupOfGw->deviceList.size(); j++)
					{
						if (groupOfGw->DelDevice(groupOfGw->deviceList[j]->device, groupOfGw->deviceList[j]->device->GetAddr()))
						{
							database->DeviceInGroupDel(groupOfGw, groupOfGw->deviceList[j]->device, groupOfGw->deviceList[j]->device->GetAddr());
							groupJsonRsp["SUCCESS"].append(groupOfGw->deviceList[j]->device->GetId());
						}
						else
						{
							hasDeviceDelGroupFailed = true;
							groupJsonRsp["FAILED"].append(groupOfGw->deviceList[j]->device->GetId());
						}
					}
					if (!hasDeviceDelGroupFailed)
					{
						database->GroupDel(groupOfGw);
					}
				}
				else
				{
					LOGW("Group %s does not exsit", groupId.c_str());
				}
				dataJsonRsp["GROUPS"] = groupJsonRsp;
				if (groupJsonRsp.isMember("SUCCESS"))
					groupJsonRsp.removeMember("SUCCESS");
				if (groupJsonRsp.isMember("FAILED"))
					groupJsonRsp.removeMember("FAILED");
				groupJsonRsp["FAILED"] = Json::arrayValue;
			}

			for (Json::ArrayIndex n = 0; n < scenesDelRoom.size(); n++)
			{
				string sceneId = scenesDelRoom[n].asString();
				sceneJsonRsp["SCENE_ID"] = sceneId;
				SceneBle *sceneOfGw = getSceneBleFromId(sceneId);
				if (sceneOfGw)
				{
					hasDeviceDelSceneFailed = false;
					for (int m = 0; m < sceneOfGw->deviceList.size(); m++)
					{
						if (sceneOfGw->DelDevice(sceneOfGw->deviceList[m]->device))
						{
							database->DeviceInSceneBleDel(sceneOfGw, sceneOfGw->deviceList[m]->device, 0);
							sceneJsonRsp["SUCCESS"].append(sceneOfGw->deviceList[m]->device->GetId());
						}
						else
						{
							sceneJsonRsp["FAILED"].append(sceneOfGw->deviceList[m]->device->GetId());
						}
					}
				}
				else
				{
					LOGW("Scene %s does not exsit", sceneId.c_str());
				}
				dataJsonRsp["SCENES"].append(sceneJsonRsp);
				if (sceneJsonRsp.isMember("SUCCESS"))
					sceneJsonRsp.removeMember("SUCCESS");
				if (sceneJsonRsp.isMember("FAILED"))
					sceneJsonRsp.removeMember("FAILED");
				sceneJsonRsp["FAILED"] = Json::arrayValue;
			}
			respValue["DATA"] = dataJsonRsp;
			if (roomId != "" && roomUnicast != 0)
			{
				Room *room = getRoomFromId(roomId);
				if (!room)
				{
					room = new Room(roomId, roomUnicast);
					room = gateway->AddNewRoom(room);
					database->RoomAdd(room);
				}
				room->DataConfigAdd(respValue.toString());
				string ruleStr = respValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->DataRoomAdd(roomId, respValue.toString());
			}
		}
	}
	else
	{
		LOGW("OnRPCDeleteRoom msg error");
	}
	return 0;
}

int Gateway::OnRPCCheckRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("Check room")
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("ROOM_ID") && data["ROOM_ID"].isString())
		{
			string roomId = data["ROOM_ID"].asString();
			if (roomId != "")
			{
				Room *room = getRoomFromId(roomId);
				if (room)
				{
					listMsgPush = room->dataConfig;
					return 2;
				}
			}
		}
	}
	return 0;
}

int Gateway::OnRPCAddGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddGroup %s", reqValue.toString().c_str());
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
				dataValue.isMember("NAME") && dataValue["NAME"].isString())
		{
			string groupId = dataValue["GROUP_ID"].asString();
			string groupName = dataValue["NAME"].asString();
			int temp_groupUnicastId = 1;
			for (auto &x : groupList)
			{
				if (x.first >= temp_groupUnicastId)
				{
					temp_groupUnicastId = x.first + 1;
				}
			}
			Group *group = new Group(groupId, temp_groupUnicastId, groupName);
			if (group)
			{
				if (AddNewGroup(group, true, true))
				{
					respValue["CMD"] = "CREATE_GROUP";
					Json::Value data;
					data["GROUP_ID"] = groupId;
					if (dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
					{
						Json::Value devices = dataValue["DEVICES"];
						for (Json::ArrayIndex i = 0; i < devices.size(); i++)
						{
							string devcieId = devices[i].asString();
							Device *device = getDeviceFromId(devcieId);
							if (device)
							{
								int tempDeviceAddr = device->GetAddr();
								if (group->AddDevice(device, tempDeviceAddr, true))
								{
									database->DeviceInGroupAdd(group, device, tempDeviceAddr);
									data["SUCCESS"].append(device->GetId());
									// respValue["code"] = 0;
									// return 0;
								}
								else
								{
									data["FAILED"].append(device->GetId());
								}
							}
						}
						// respValue["code"] = 0;
						// return 0;
						respValue["DATA"] = data;
					}
				}
				else
				{
					delete group;
				}
			}
		}
	}
	return 0;
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
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString())
		{
			respValue["CMD"] = "DELETE_GROUP";
			Json::Value data;
			string groupId = dataValue["GROUP_ID"].asString();
			data["GROUP_ID"] = groupId;
			Group *group = getGroupFromId(groupId);
			if (group)
			{
				int temp_groupUnicastId = group->GetId();
				bool hasDeviceDelGroupFailed = false;
				int numberDevOfGroup = group->deviceList.size();
				vector<Device *> tempDevInGroup;
				for (int i = 0; i < numberDevOfGroup; i++)
				{
					tempDevInGroup.push_back(group->deviceList[i]->device);
				}
				for (int j = 0; j < numberDevOfGroup; j++)
				{
					if (group->DelDevice(tempDevInGroup[j], tempDevInGroup[j]->GetAddr()))
					{
						database->DeviceInGroupDel(group, tempDevInGroup[j], tempDevInGroup[j]->GetAddr());
						data["SUCCESS"].append(tempDevInGroup[j]->GetId());
					}
					else
					{
						hasDeviceDelGroupFailed = true;
						data["FAILED"].append(tempDevInGroup[j]->GetId());
					}
				}
				if (!hasDeviceDelGroupFailed)
				{
					database->GroupDel(group);
				}

				// respValue["code"] = 0;
				// return 0;
			}
			respValue["DATA"] = data;
		}
	}
	// respValue["code"] = -1;
	return 0;
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
				respValue["CMD"] = "ADD_DEVICE_TO_GROUP";
				Json::Value data;
				string groupId = dataValue["GROUP_ID"].asString();
				data["GROUP_ID"] = groupId;
				Json::Value deviceList = dataValue["DEVICES"];
				Group *group = getGroupFromId(groupId);
				if (group)
				{
					for (int i = 0; i < (int)deviceList.size(); i++)
					{
						string devcieId = deviceList[i].asString();
						Device *device = getDeviceFromId(devcieId);
						if (device)
						{
							int tempDeviceAddr = device->GetAddr();
							if (group->AddDevice(device, tempDeviceAddr, true))
							{
								database->DeviceInGroupAdd(group, device, tempDeviceAddr);
								data["SUCCESS"].append(device->GetId());
								// respValue["code"] = 0;
								// return 0;
							}
							else
							{
								data["FAILED"].append(device->GetId());
							}
						}
					}
				}
				else
				{
					LOGW("Group id: %s does not exist", groupId.c_str());
				}
				respValue["DATA"] = data;
			}
		}
		// respValue["code"] = -1;
		// return -1;
	}
	catch (const char *msg)
	{
		LOGE("OnRPCAddDeviceToGroup fail");
		return -1;
	}
	return 0;
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
			respValue["CMD"] = "DELETE_DEVICE_FROM_GROUP";
			Json::Value data;
			string groupId = dataValue["GROUP_ID"].asString();
			data["GROUP_ID"] = groupId;
			Group *group = getGroupFromId(groupId);
			if (group)
			{
				Json::Value deviceList = dataValue["DEVICES"];
				for (int i = 0; i < (int)deviceList.size(); i++)
				{
					string deviceId = deviceList[i].asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						int tempDeviceAddr = device->GetAddr();
						if (group->DelDevice(device, tempDeviceAddr))
						{
							database->DeviceInGroupDel(group, device, tempDeviceAddr);
							data["SUCCESS"].append(device->GetId());
							// respValue["code"] = 0;
							// return 0;
						}
						else
						{
							data["FAILED"].append(device->GetId());
						}
					}
				}
			}
			else
			{
				LOGW("Group id: %s does not exist", groupId.c_str());
			}
			respValue["DATA"] = data;
		}
	}
	// respValue["code"] = -1;
	return 0;
}

static int GetIdButton(string button)
{
	string listButtonId[] = {"BUTTON_1", "BUTTON_2", "BUTTON_3", "BUTTON_4", "BUTTON_5", "BUTTON_6"};
	for (int i = 0; i < 6; i++)
	{
		if (listButtonId[i].compare(button) == 0)
		{
			return (i + 1);
		}
	}
	return -1;
}

int Gateway::OnRPCSetSceneForRemote(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "SCENE_FOR_REMOTE";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() && dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString() && dataValue.isMember("BUTTON_VALUE") && dataValue["BUTTON_VALUE"].isString() && dataValue.isMember("MODE_VALUE") && dataValue["MODE_VALUE"].isInt())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			string buttonValue = dataValue["BUTTON_VALUE"].asString();
			int buttonId = GetIdButton(buttonValue);
			string sceneId = dataValue["SCENE_ID"].asString();
			int modeValue = dataValue["MODE_VALUE"].asInt();
			dataJsonRsp["DEVICE_ID"] = deviceId;
			dataJsonRsp["BUTTON_VALUE"] = buttonValue;
			dataJsonRsp["MODE_VALUE"] = modeValue;
			dataJsonRsp["SCENE_ID"] = sceneId;
			respValue["DATA"] = dataJsonRsp;
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				SceneBle *scene = getSceneBleFromId(sceneId);
				if (scene)
				{
					if (device->GetType() == BLE_DC_SCENE_CONTACT)
					{
						if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), buttonId, modeValue, scene->GetId(), 0) == 0)
						{
							return 0;
						}
					}
					else if (device->GetType() == BLE_AC_SCENE_CONTACT)
					{
						if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), buttonId, modeValue, scene->GetId(), 0) == 0)
						{
							return 0;
						}
					}
				}
				else
				{
					LOGW("Scene %s does not exsit", sceneId.c_str())
				}
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
	}
	return -1;
}

int Gateway::OnRPCDelSceneForRemote(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "DELETE_SCENE_FOR_REMOTE";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() && dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString() && dataValue.isMember("BUTTON_VALUE") && dataValue["BUTTON_VALUE"].isString() && dataValue.isMember("MODE_VALUE") && dataValue["MODE_VALUE"].isInt())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			string buttonValue = dataValue["BUTTON_VALUE"].asString();
			int buttonId = GetIdButton(buttonValue);
			string sceneId = dataValue["SCENE_ID"].asString();
			int modeValue = dataValue["MODE_VALUE"].asInt();
			dataJsonRsp["DEVICE_ID"] = deviceId;
			dataJsonRsp["BUTTON_VALUE"] = buttonValue;
			dataJsonRsp["MODE_VALUE"] = modeValue;
			dataJsonRsp["SCENE_ID"] = sceneId;
			respValue["DATA"] = dataJsonRsp;
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (device->GetType() == BLE_DC_SCENE_CONTACT)
				{
					if (bleProtocol->DelSceneSwitchSceneDC(device->GetAddr(), buttonId, modeValue) == 0)
					{
						return 0;
					}
				}
				else if (device->GetType() == BLE_AC_SCENE_CONTACT)
				{
					if (bleProtocol->DelSceneSwitchSceneAC(device->GetAddr(), buttonId, modeValue) == 0)
					{
						return 0;
					}
				}
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
	}
	return -1;
}

int Gateway::OnRPCResetRemote(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "RESET_REMOTE";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			dataJsonRsp["DEVICE_ID"] = deviceId;
			respValue["DATA"] = dataJsonRsp;
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (device->GetType() == BLE_DC_SCENE_CONTACT)
				{
					for (int i = 1; i <= 6; i++)
					{
						if (bleProtocol->DelSceneSwitchSceneDC(device->GetAddr(), i, 0))
						{
							LOGW("del scene error button %d, mode 0", i);
						}
						if (bleProtocol->DelSceneSwitchSceneDC(device->GetAddr(), i, 1))
						{
							LOGW("del scene error button %d, mode 1", i);
						}
					}
				}
				else if (device->GetType() == BLE_AC_SCENE_CONTACT)
				{
					for (int j = 1; j <= 6; j++)
					{
						if (bleProtocol->DelSceneSwitchSceneAC(device->GetAddr(), j, 0))
						{
							LOGW("del scene error button %d, mode 0", j);
						}
						if (bleProtocol->DelSceneSwitchSceneAC(device->GetAddr(), j, 1))
						{
							LOGW("del scene error button %d, mode 1", j);
						}
					}
				}
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
	}
	return 0;
}

int Gateway::OnRPCScenePirLigtSensor(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCScenePirLigtSensor");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["CMD"] = "SCENE_FOR_SENSOR_LIGHT_PIR";
		Json::Value data = reqValue["DATA"];
		if (data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString())
		{
			string deviceId = data["DEVICE_ID"].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (device->GetType() == BLE_PIR_LIGHT_SENSOR_DC)
				{
					if (data.isMember("SCENE_ID") && data["SCENE_ID"].isString() && data.isMember("LUX") && data["LUX"].isArray() && data.isMember("PIR_VALUE") && data["PIR_VALUE"].isInt())
					{
						string sceneId = data["SCENE_ID"].asString();
						uint8_t pir = data["PIR_VALUE"].asInt();
						data["EVENT_TRIGGER_ID"] = sceneId;
						Json::Value lux = data["LUX"];
						SceneBle *scene = getSceneBleFromId(sceneId);
						if (scene)
						{
							Json::Value dataCmd;
							dataCmd["pir"] = pir;
							dataCmd["scene"] = scene->GetId();
							dataCmd["lux"] = lux;
							device->Do(dataCmd);
						}
						else
						{
							LOGW("Scene %s does not exsit", sceneId.c_str());
						}
					}
					if (data.isMember("HANG_ON_TIME") && data["HANG_ON_TIME"].isInt())
					{
						Json::Value configTime;
						configTime["time"] = data["HANG_ON_TIME"].asInt();
						device->Do(configTime);
					}
				}
				else if (device->GetType() == BLE_PIR_LIGHT_SENSOR_AC || device->GetType() == BLE_PIR_LIGHT_SENSOR_AC_AMTRAN)
				{
					device->Do(data);
				}
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
		dataJsonRsp["DATA"] = data;
	}
	return -1;
}
int Gateway::OnRPCEditScenePirLightSensor(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCEditScenePirLightSensor");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["CMD"] = "EDIT_SCENE_FOR_SENSOR_LIGHT_PIR";
		Json::Value data = reqValue["DATA"];
		if (data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString())
		{
			string deviceId = data["DEVICE_ID"].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (data.isMember("SCENE_ID") && data["SCENE_ID"].isString() && data.isMember("LUX") && data["LUX"].isArray() && data.isMember("PIR_VALUE") && data["PIR_VALUE"].isInt())
				{
					string sceneId = data["SCENE_ID"].asString();
					uint8_t pir = data["PIR_VALUE"].asInt();
					data["EVENT_TRIGGER_ID"] = sceneId;
					Json::Value lux = data["LUX"];
					SceneBle *scene = getSceneBleFromId(sceneId);
					if (scene)
					{
						if (device->GetType() == BLE_PIR_LIGHT_SENSOR_DC)
						{
							Json::Value dataCmd;
							dataCmd["pir"] = pir;
							dataCmd["scene"] = scene->GetId();
							dataCmd["lux"] = lux;
							device->Do(dataCmd);
						}
						else if (device->GetType() == BLE_PIR_LIGHT_SENSOR_AC || device->GetType() == BLE_PIR_LIGHT_SENSOR_AC_AMTRAN)
						{
							bleProtocol->SetScenePirLightSensor(device->GetAddr(), 2, pir, lux[0].asInt(), lux[1].asInt(), scene->GetId(), 1);
						}
					}
					else
					{
						LOGW("Scene %s does not exsit", sceneId.c_str());
					}
				}
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
		dataJsonRsp["DATA"] = data;
	}
	return -1;
}
int Gateway::OnRPCRemoveScenePirLightSensor(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCRemoveScenePirLightSensor");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isArray() && reqValue.isMember("DEVICE_ID") && reqValue["DEVICE_ID"].isString())
	{
		respValue["CMD"] = "REMOVE_SCENE_FOR_SENSOR_LIGHT_PIR";
		Json::Value data = reqValue["DATA"];
		string deviceId = reqValue["DEVICE_ID"].asString();
		for (Json::ArrayIndex i = 0; i < data.size(); i++)
		{
			Json::Value dataValue = data[i];
			if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
			{
				string eventId = dataValue["EVENT_TRIGGER_ID"].asString();
				Device *device = getDeviceFromId(deviceId);
				SceneBle *scene = getSceneBleFromId(eventId);
				if (device && scene)
				{
					if (device->GetType() == BLE_PIR_LIGHT_SENSOR_DC)
					{
						Json::Value delscene;
						delscene["sceneDel"] = scene->GetId();
						device->Do(delscene);
					}
					else
					{
						device->Do(data);
					}
				}
				else
				{
					LOGW("Device or scene does exsit");
				}
			}
		}
		respValue["DATA"] = data;
		return 0;
	}
	LOGW("OnRPCRemoveScenePirLightSensor error: %s", reqValue.toString().c_str());
	return -1;
}

int Gateway::OnRPCSceneScreen(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "SCENE_FOR_SCREEN";
		Json::Value data = reqValue["DATA"];
		Json::Value dataJson = Json::objectValue;
		if (data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString())
		{
			string deviceId = data["DEVICE_ID"].asString();
			dataJson["DEVICE_ID"] = deviceId;
			Device *device = getDeviceFromId(deviceId);
			string status = "SUCCESS";
			if (device && device->GetType() == BLE_AC_SCENE_SCREEN_TOUCH)
			{
				if (data.isMember("SCENES") && data["SCENES"].isArray())
				{
					Json::Value scenes = data["SCENES"];
					for (Json::ArrayIndex i = 0; i < scenes.size(); i++)
					{
						Json::Value scene = scenes[i];
						if (scene.isMember("SCENE_ID") && scene["SCENE_ID"].isString() && scene.isMember("SCENE_NAME") && scene["SCENE_NAME"].isString() && scene.isMember("SCENE_ICON") && scene["SCENE_ICON"].isInt())
						{
							string sceneId = scene["SCENE_ID"].asString();
							string sceneName = scene["SCENE_NAME"].asString();
							int sceneIcon = scene["SCENE_ICON"].asInt();
							SceneBle *scene = getSceneBleFromId(sceneId);
							if (scene)
							{
								if (bleProtocol->SceneForScreenTouch(device->GetAddr(), scene->GetId(), sceneIcon, 1) != 0)
								{
									status = "FAILED";
								}
							}
							else
							{
								LOGW("Scene %s not found", sceneId.c_str());
							}
						}
					}
				}

				if (data.isMember("DEL_SCENES") && data["DEL_SCENES"].isArray())
				{
					Json::Value delScene = data["DEL_SCENES"];
					for (Json::ArrayIndex j = 0; j < delScene.size(); j++)
					{
						if (delScene[j].isString())
						{
							string sceneId = delScene[j].asString();
							SceneBle *sceneDel = getSceneBleFromId(sceneId);
							if (sceneDel)
							{
								if (bleProtocol->DelSceneScreenTouch(device->GetAddr(), sceneDel->GetId()) != 0)
								{
									status = "FAILED";
								}
							}
							else
							{
								LOGW("Scene del %s not found", sceneId.c_str());
							}
						}
					}
				}
				dataJson["STATUS"] = status;
			}
			else
			{
				LOGW("Device %s is not supported", deviceId.c_str());
			}
		}
		respValue["DATA"] = dataJson;
		return 0;
	}
	LOGW("OnRPCRemoveScenePirLightSensor error: %s", reqValue.toString().c_str());
	return -1;
}

/*
{
	"CMD" : "STAIRS_SWITCH",
	"DATA":
	{
		"DEVICE_ID" : "aaaaaaa-6f18-43c0-ae46-69c32998f653",
		"LIST_BUTTON_LINK" : [
			1,
			2,
			3,
			4
		]
	}
}

{
	"CMD": "STAIRS_SWITCH",
	"DATA": {
	"DEVICE_ID": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
	"SUCCESS": [
		1,
		2
	],
	"FAILED": [
		3,
		4
	]
	}
}
*/
int Gateway::OnRPCStairsSwitch(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "STAIRS_SWITCH";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value data = reqValue["DATA"];
		if (data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString() && data.isMember("LIST_BUTTON_LINK") && data["LIST_BUTTON_LINK"].isArray())
		{
			string deviceId = data["DEVICE_ID"].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				for (Json::ArrayIndex i = 0; i < data["LIST_BUTTON_LINK"].size(); i++)
				{
					int button = data["LIST_BUTTON_LINK"][i].asInt();
					string groupRandom = Util::genRandRQI(16);
					int temp_groupUnicastId = 1;
					for (auto &x : groupList)
					{
						if (x.first >= temp_groupUnicastId)
						{
							temp_groupUnicastId = x.first + 1;
						}
					}
					Group *group = new Group(groupRandom, temp_groupUnicastId, groupRandom);
					if (group)
					{
						if (AddNewGroup(group, true, true))
						{
							if (group->AddDevice(device, device->GetAddr() + button, true))
							{
								database->DeviceInGroupAdd(group, device, device->GetAddr() + (button - 1));
								dataJsonRsp["SUCCESS"].append(button);
							}
							else
							{
								dataJsonRsp["FAILED"].append(button);
							}
						}
						else
						{
							delete group;
						}
					}
				}
			}
			else
			{
				LOGW("Device %s not found", deviceId.c_str());
			}
		}
		respValue["DATA"] = dataJsonRsp;
	}
	return 0;
}

/**
 *
{
	"CMD": "EDIT_STAIRS_SWITCH",
	"DATA": {
	"DEVICE_ID": "aaaaaaa-6f18-43c0-ae46-69c32998f653",
	"ADD_BUTTON": [
		1,
		2
	],
	"REMOVE_BUTTON": [
		3,
		4
	]
	}
}

{
	"CMD": "EDIT_STAIRS_SWITCH",
	"DATA": {
	"DEVICE_ID": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
	"SUCCESS": [
		1,
		2
	],
	"FAILED": [
		3,
		4
	]
	}
}
*/
int Gateway::OnRPCEditStairsSwitch(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value &data = reqValue["DATA"];
		respValue["CMD"] = "EDIT_STAIRS_SWITCH";
		Json::Value dataJsonRsp = Json::objectValue;
		if (data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString() && data.isMember("ADD_BUTTON") && data["ADD_BUTTON"].isArray() && data.isMember("REMOVE_BUTTON") && data["REMOVE_BUTTON"].isArray())
		{
			string deviceId = data["DEVICE_ID"].asString();
			uint16_t groupMesh;
			Device *device = getDeviceFromId(deviceId);
			Group *group = NULL;
			if (device)
			{
				for (int i = 0; i < groupList.size(); i++)
				{
					for (int j = 0; j < groupList[i]->deviceList.size(); j++)
					{
						if (groupList[i]->deviceList[j]->device->GetId() == deviceId)
						{
							group = groupList[i];
							groupMesh = groupList[i]->GetId();
							break;
						}
					}
				}
				Json::Value buttonsAdd = data["ADD_BUTTON"];
				for (int n = 0; n < buttonsAdd.size(); n++)
				{
					if (buttonsAdd[n].isInt())
					{
						int button = buttonsAdd[n].asInt();
						if (group->AddDevice(device, device->GetAddr() + button, true))
						{
							database->DeviceInGroupAdd(group, device, device->GetAddr() + (button - 1));
							dataJsonRsp["SUCCESS"].append(button);
						}
						else
						{
							dataJsonRsp["FAILED"].append(button);
						}
					}
				}
				Json::Value buttonsRemove = data["REMOVE_BUTTON"];
				for (int m = 0; m < buttonsRemove.size(); m++)
				{
					if (buttonsRemove[m].isInt())
					{
						int buttonRv = buttonsRemove[m].asInt();
						if (group->DelDevice(device, device->GetAddr() + buttonRv))
						{
							database->DeviceInGroupDel(group, device, device->GetAddr() + (buttonRv - 1));
							dataJsonRsp["SUCCESS"].append(buttonRv);
						}
						else
						{
							dataJsonRsp["FAILED"].append(buttonRv);
						}
					}
				}
			}
			else
			{
				LOGW("Device %s not found", deviceId.c_str());
			}
		}
		respValue["DATA"] = dataJsonRsp;
	}
	return 0;
}

/**
{
	"CMD": "DELETE_STAIRS_SWITCH",
	"DATA": {
	"DEVICE_ID": "aaaaaaa-6f18-43c0-ae46-69c32998f653"
	}
}

{
	"CMD": "DELETE_STAIRS_SWITCH",
	"DATA": {
	"DEVICE_ID": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
	"STATUS": "SUCCESS"
	}
}

*/
int Gateway::OnRPCDelStairsSwitch(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value &data = reqValue["DATA"];
		respValue["CMD"] = "DELETE_STAIRS_SWITCH";
		Json::Value dataJsonRsp = Json::objectValue;
		if (data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString())
		{
			string deviceId = data["DEVICE_ID"].asString();
			dataJsonRsp["DEVICE_ID"] = deviceId;
			Device *device = getDeviceFromId(deviceId);
			uint16_t groupMesh;
			Group *group = NULL;
			for (int i = 0; i < groupList.size(); i++)
			{
				for (int j = 0; j < groupList[i]->deviceList.size(); j++)
				{
					if (groupList[i]->deviceList[j]->device->GetId() == deviceId)
					{
						group = groupList[i];
						groupMesh = groupList[i]->GetId();
						break;
					}
				}
			}
			if (device)
			{
				int numberButtons = 0;
				uint16_t type = device->GetType();
				switch (type)
				{
				case BLE_SWITCH_RGB_1:
					numberButtons = 1;
					break;
				case BLE_SWITCH_RGB_2:
					numberButtons = 2;
					break;
				case BLE_SWITCH_RGB_3:
					numberButtons = 3;
					break;
				case BLE_SWITCH_RGB_4:
					numberButtons = 4;
					break;
				}
				for (int i = 0; i < numberButtons; i++)
				{
					if (group->DelDevice(device, device->GetAddr() - i))
					{
						database->DeviceInGroupDel(group, device, device->GetAddr() - i);
					}
				}
			}
			else
			{
				LOGW("Device %s not found", deviceId.c_str());
			}
		}
		dataJsonRsp["STATUS"] = "SUCCESS";
		respValue["DATA"] = dataJsonRsp;
	}
	return 0;
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
				dataValue.isMember("type") && dataValue["type"].isInt() &&
				dataValue.isMember("devicekey") && dataValue["devicekey"].isString() &&
				dataValue.isMember("version") && dataValue["version"].isInt())
		{
			string deviceId = dataValue["id"].asString();
			string name = dataValue["name"].asString();
			string mac = dataValue["mac"].asString();
			uint32_t addr = dataValue["addr"].asInt();
			uint32_t type = dataValue["type"].asInt();
			string devicekey = dataValue["devicekey"].asString();
			uint16_t version = dataValue["version"].asInt();
			Device *device = AddNewDevice(deviceId, name, mac, devicekey, addr, type, version, true, true);
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
			// 	for (Json::ArrayIndex i = 0; i < properties.size(); i++)
			// 	{
			// 		Json::Value property = properties[i];
			// 		if (property.isObject() &&
			// 				property.isMember("ID") && property["ID"].isInt() &&
			// 				property.isMember("CMD") && property["CMD"].isString())
			// 		{
			// 			int id = property["ID"].asInt();
			// 			string cmd = property["CMD"].asString();
			// 			device->DoJsonArrayDo(id, value);
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
		scanDeviceValue["type"] = (int)scanDevice->GetType();
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
				if (properties.isArray())
				{
					int hsl = 0;
					int rgbDimOnOff = 0;
					for (Json::ArrayIndex i = 0; i < properties.size(); i++)
					{
						Json::Value property = properties[i];
						if (property.isObject() &&
								property.isMember("ID") && property["ID"].isInt())
						{
							if (property["ID"].asInt() == BLE_ATTRIBUTE_HUE || property["ID"].asInt() == BLE_ATTRIBUTE_SATURATION || property["ID"].asInt() == BLE_ATTRIBUTE_LUMINANCE)
							{
								hsl++;
							}
							else if (property["ID"].asInt() == BLE_ATTRIBUTE_R || property["ID"].asInt() == BLE_ATTRIBUTE_B || property["ID"].asInt() == BLE_ATTRIBUTE_G || property["ID"].asInt() == BLE_ATTRIBUTE_DIM_OFF || property["ID"].asInt() == BLE_ATTRIBUTE_DIM_ON)
							{
								rgbDimOnOff++;
							}
						}
					}
					if (hsl == 3 || rgbDimOnOff == 5)
					{
						device->Do(properties);
					}
					else
					{
						device->DoJsonArray(properties);
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
				if (properties.isArray())
				{
					group->Do(properties);
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

int Gateway::OnRPCControlSceneBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString())
		{
			string sceneId = dataValue["SCENE_ID"].asString();
			SceneBle *scene = getSceneBleFromId(sceneId);
			if (scene)
			{
				scene->Do(scene->GetId());
			}
			else
			{
				LOGW("Scene %s dose not exsit", sceneId.c_str());
			}
		}
	}
	return 0;
}

int Gateway::OnRPCSetPwMqttOnline(Json::Value &reqValue, Json::Value &respValue)
{
// TODO: add for esp platform
#ifndef ESP_PLATFORM
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "SET_PASSWD_MQTT_ONLINE";
		Json::Value dataJsonRsp = Json::objectValue;
		int status = 0;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("PASSWD") && dataValue["PASSWD"].isString())
		{
			string password = dataValue["PASSWD"].asString();
			if (config->GetPassword() == "")
			{
				string user = "hc-" + mac;
				if (config->SetClientId(user))
				{
					if (config->SetUsername(user))
					{
						if (config->SetPassword(password))
						{
							status = 1;
						}
						else
						{
							status = 0;
						}
					}
					else
					{
						status = 0;
					}
				}
				else
				{
					status = 0;
				}
				dataJsonRsp["STATUS"] = status;
				respValue["DATA"] = dataJsonRsp;
				return -10;
			}
			else
			{
				dataJsonRsp["STATUS"] = 0;
				respValue["DATA"] = dataJsonRsp;
				return 0;
			}
		}
	}
#endif
	return -1;
}

int Gateway::OnRPCSSHRemote(Json::Value &reqValue, Json::Value &respValue)
{
	int err = 0;
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
				string decode = macaron::Base64::Decode(keyBase64, key);
				if (decode != "")
				{
					err = 1;
					LOGW("Base64 decode err: %s", decode.c_str());
				}
			}
			else
			{
				key = dataValue["key"].asString();
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
				return 0;
			}
		}
	}
	respValue["code"] = err;
	return 0;
}

int Gateway::OnRPCAddDeviceSmartHomeToRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() &&
				dataValue.isMember("ROOM_ID") && dataValue["ROOM_ID"].isString())
		{
			int deviceType;
			string deviceId = dataValue["DEVICE_ID"].asString();
			string roomId = dataValue["ROOM_ID"].asString();
			DeviceBle *device = (DeviceBle *)gateway->getDeviceFromId(deviceId);
			Group *group = getGroupFromId(roomId);
			if (group)
			{
				Room *room = getRoomFromId(roomId);
				if (room)
				{
					if (device)
					{
						deviceType = device->GetType();
						if (deviceType == BLE_REMOTE_M3_V2 || deviceType == BLE_REMOTE_M4 || deviceType == BLE_AC_SCENE_SCREEN_TOUCH)
						{
							if (room->AddDevice(device, true))
							{
								database->DeviceInRoomAdd(room, device);
							}
						}
						else
						{
							LOGW("Device type does not support add room");
						}
					}
					else
					{
						LOGW("Device %s is does not exist", deviceId.c_str());
					}
				}
				else
				{
					room = new Room(roomId, group->GetId());
					if (room)
					{
						Room *roomAddGw = AddNewRoom(room);
						if (roomAddGw)
						{
							if (device)
							{
								deviceType = device->GetType();
								if (deviceType == BLE_REMOTE_M3_V2 || deviceType == BLE_REMOTE_M4 || deviceType == BLE_AC_SCENE_SCREEN_TOUCH)
								{
									if (roomAddGw->AddDevice(device, true))
									{
										database->DeviceInRoomAdd(roomAddGw, device);
									}
								}
								else
								{
									LOGW("Device type does not support add room");
								}
							}
							else
							{
								LOGW("Device %s is does not exist", deviceId.c_str());
							}
						}
					}
				}
			}
			else
			{
				LOGW("room %s does not exist", roomId.c_str());
			}
		}
	}
	return 0;
}

int Gateway::OnRPCCreateCountDown(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "COUNTDOWN";
		Json::Value dataValue = reqValue["DATA"];
		respValue["DATA"] = dataValue;
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString() && dataValue.isMember("START_AT") && dataValue["START_AT"].isString() && dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString())
		{
			string eventTriggerId = dataValue["EVENT_TRIGGER_ID"].asString();
			string startAt = dataValue["START_AT"].asString();
			string sceneId = dataValue["SCENE_ID"].asString();
			SceneBle *scene = getSceneBleFromId(sceneId);
			Rule *rule = getRuleById(eventTriggerId);
			if (rule)
			{
				rule->DelAllRuleInput();
				rule->DelAllRuleOutput();
			}
			if (scene)
			{
				int day = Util::GetDaysCurrent();
				int mon = 0, tue = 0, wed = 0, thu = 0, fri = 0, sat = 0, sun = 0;
				switch (day)
				{
				case 2:
					mon = 1;
					break;
				case 3:
					tue = 1;
					break;
				case 4:
					wed = 1;
					break;
				case 5:
					thu = 1;
					break;
				case 6:
					fri = 1;
					break;
				case 7:
					sat = 1;
					break;
				case 8:
					sun = 1;
					break;
				}
				int repeat = Util::ConvertRepeatDayToInt(mon, tue, wed, thu, fri, sat, sun);
				rule = new Rule(eventTriggerId, "and", repeat, Util::ConvertStrTimeToInt(startAt), Util::ConvertStrTimeToInt(""));
				RuleOutputSceneBle *ruleOutputSceneBle = new RuleOutputSceneBle(scene);
				rule->AddRuleOutput(ruleOutputSceneBle);
				ruleList[eventTriggerId] = rule;
			}
			else
			{
				LOGW("Scene %s not found", sceneId.c_str());
			}
			return 0;
		}
		else
		{
			LOGW("CountDown failed");
		}
	}
	return -1;
}

int Gateway::OnRPCDelCountDown(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "DELETE_COUNTDOWN";
		Json::Value dataValue = reqValue["DATA"];
		respValue["DATA"] = dataValue;
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string ruleId = dataValue["EVENT_TRIGGER_ID"].asString();
			ruleList.erase(ruleList.find(ruleId));
			return 0;
		}
		return 0;
	}
	return -1;
}

int Gateway::OnRPCUpdateFirmware(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCUpdateFirmware");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isArray())
	{
		Json::Value datasValue = reqValue["DATA"];
		for (Json::ArrayIndex i = 0; i < datasValue.size(); i++)
		{
			Json::Value dataValue = datasValue[0];
			if (dataValue.isMember("NAME") && dataValue["NAME"].isString() &&
					dataValue.isMember("CHECK_SUM") && dataValue["CHECK_SUM"].isString() &&
					dataValue.isMember("URL") && dataValue["URL"].isString())
			{
				string name = dataValue["NAME"].asString();
				string sum = dataValue["CHECK_SUM"].asString();
				string url = dataValue["URL"].asString();
				Ota::startOta(name, url, sum);
				return 0;
			}
		}
	}
	else
	{
		LOGW("Format error");
	}
	return 0;
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
	dataValue["DEVICE_UNICAST_ID"] = (int)scanDevice->GetAddr();
	dataValue["DEVICE_TYPE_ID"] = (int)scanDevice->GetType();
	dataValue["MAC_ADDRESS"] = scanDevice->GetMac();
	dataValue["FIRMWARE_VERSION"] = scanDevice->GetVersionStr();
	dataValue["DEVICE_KEY"] = scanDevice->GetDeviceId();
	dataValue["NET_KEY"] = gateway->getBleNetkey();
	dataValue["APP_KEY"] = gateway->getBleAppKey();
	jsonValue["CMD"] = "NEW_DEVICE";
	jsonValue["DATA"] = dataValue;
#ifdef CONFIG_USE_OLD_APP
	PublishToLocalMessage(jsonValue);
#else
	PublishToDeviceTelemetry(jsonValue);
#endif
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
		if (group->GetUUId() == groupId)
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
		if (device->CheckAddr(addr) && device->GetProtocol() == BLE_DEVICE)
		{
			DeviceBle *deviceBle = dynamic_cast<DeviceBle *>(device);
			if (deviceBle)
				return deviceBle;
		}
	}
	return NULL;
}

Rule *Gateway::getRuleById(string eventId)
{
	for (const auto &[id, rule] : ruleList)
	{
		if (rule->GetId() == eventId)
		{
			return rule;
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

SceneBle *Gateway::getSceneBleFromId(string sceneBleUUId)
{
	for (const auto &[id, scene] : sceneBleList)
	{
		if (scene->GetUUId() == sceneBleUUId)
			return scene;
	}
	return NULL;
}

Room *Gateway::getRoomFromId(string roomUUId)
{
	for (auto &[id, room] : roomList)
	{
		if (id == roomUUId)
			return room;
	}
	return NULL;
}

Device *Gateway::AddNewDevice(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version, bool addGateway, bool addDatabase)
{
	LOGI("Add new device id: %s, name: %s, mac: %s, addr: 0x%04X, type: 0x%04X, verion: %d", id.c_str(), name.c_str(), mac.c_str(), addr, type, version);
	Device *device = NULL;
	switch (type)
	{
	case BLE_LED_CHIEU_TRANH:
	case BLE_LED_CHIEU_GUONG:
	case BLE_DEN_BAN:
	case BLE_DOWNLIGHT_SMT:
	case BLE_DOWNLIGHT_COB_GOC_HEP:
	case BLE_DOWNLIGHT_COB_GOC_RONG:
	case BLE_DOWNLIGHT_COB_TRANG_TRI:
	case BLE_LED_FLOOD:
	case BLE_LED_DAY_LINEAR:
	case BLE_LED_OP_TRAN:
	case BLE_LED_OP_TUONG:
	case BLE_LED_OP_TRAN_LOA:
	case BLE_PANEL_TRON:
	case BLE_PANEL_VUONG:
	case BLE_TRACKLIGHT:
	case BLE_LED_THA_TRAN:
	case BLE_LED_TUBE_M16:
		device = new DeviceBleLightOnoffCctDim(id, name, mac, device_id, addr, type, version);
		break;
	case BLE_DOWNLIGHT_RGBCW:
	case BLE_LED_DAY_RGBCW:
	case BLE_LED_BULB:
		device = new DeviceBleLightOnoffCctDimHslModeRGB(id, name, mac, device_id, addr, type, version);
		break;
	case BLE_LED_DAY_RGB:
		device = new DeviceBleLightOnoffHslModeRGB(id, name, mac, device_id, addr, type, version);
		break;
	case BLE_SWITCH_RGB_4:
		device = new DeviceBleSwitchTouchRgb4(id, name, mac, device_id, addr, version);
		break;
	case BLE_DC_SCENE_CONTACT:
		device = new DeviceBleSwitchScene6DC(id, name, mac, device_id, addr, version);
		break;
	case BLE_TEMP_HUM_SENSOR:
		device = new DeviceBleSensorTempHum(id, name, mac, device_id, addr, version);
		break;
	case BLE_PM_SENSOR:
		device = new DeviceBleSensorPm(id, name, mac, device_id, addr, version);
		break;
	case BLE_PIR_LIGHT_SENSOR_DC:
		device = new DeviceBlePirLightSensorDC(id, name, mac, device_id, addr, version);
		break;
	case BLE_SMOKE_SENSOR:
		device = new DeviceBleSmokeSensor(id, name, mac, device_id, addr, version);
		break;
	case BLE_DOOR_SENSOR:
		device = new DeviceBleDoorSensor(id, name, mac, device_id, addr, version);
		break;
	case BLE_AC_SCENE_SCREEN_TOUCH:
		device = new DeviceBleScreenTouch(id, name, mac, device_id, addr, version);
		break;
	default:
		LOGW("Add new device not support type: 0x%04X", type);
		break;
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
		device->lastTimeActive = time(NULL);
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

Rule *Gateway::AddRule(Json::Value &ruleValue, bool addGateway, bool addDatabase)
{
	if (ruleValue.isMember("EVENT_TRIGGER_ID") && ruleValue["EVENT_TRIGGER_ID"].isString() &&
			ruleValue.isMember("LOGICAL_OPERATOR_ID") && ruleValue["LOGICAL_OPERATOR_ID"].isInt() &&
			ruleValue.isMember("STATUS") && ruleValue["STATUS"].isInt() &&
			ruleValue.isMember("EACH_DAY") && ruleValue["EACH_DAY"].isArray())
	{
		int status = ruleValue["STATUS"].asInt();
		string id = ruleValue["EVENT_TRIGGER_ID"].asString();
		Json::Value repeatDays = ruleValue["EACH_DAY"];
		int mon = 0, tue = 0, wed = 0, thu = 0, fri = 0, sat = 0, sun = 0;
		int repeat;
		if (repeatDays.size() > 0)
		{
			for (Json::ArrayIndex i = 0; i < repeatDays.size(); ++i)
			{
				if (repeatDays[i] == "EACHMONDAY")
					mon = 1;
				else if (repeatDays[i] == "EACHTUESDAY")
					tue = 1;
				else if (repeatDays[i] == "EACHWEDNESDAY")
					wed = 1;
				else if (repeatDays[i] == "EACHTHUSDAY")
					thu = 1;
				else if (repeatDays[i] == "EACHFRIDAY")
					fri = 1;
				else if (repeatDays[i] == "EACHSATURDAY")
					sat = 1;
				else if (repeatDays[i] == "EACHSUNDAY")
					sun = 1;
			}
			repeat = Util::ConvertRepeatDayToInt(mon, tue, wed, thu, fri, sat, sun);
		}
		else
		{
			repeat = Util::ConvertRepeatDayToInt(1, 1, 1, 1, 1, 1, 1);
		}

		Rule *rule = NULL;

		// TODO: Check Type of Rule:
		/*
			- -1: Rule Time
			- +0: Rule OR
			- +1: Rule AND
			- +2: Rule Time + OR
			- +3: Rule Time + AND
		*/
		int logical = ruleValue["LOGICAL_OPERATOR_ID"].asInt();
		string type;
		if (logical == -1 || logical == 3 || logical == 2) // rule theo thoi gian or theo thoi gian va tb dau vao
		{
			if (logical == -1 || logical == 3)
			{
				type = "and";
			}
			else if (logical == 2)
			{
				type = "or";
			}
			if (ruleValue.isMember("START_AT") && ruleValue["START_AT"].isString())
			{
				string endAt;
				if (ruleValue.isMember("END_AT") && ruleValue["END_AT"].isString())
				{
					endAt = ruleValue["END_AT"].asString();
				}
				string startAt = ruleValue["START_AT"].asString();
				rule = new Rule(id, type, repeat, Util::ConvertStrTimeToInt(startAt), Util::ConvertStrTimeToInt(endAt));
				if (!rule)
				{
					LOGW("New rule error");
				}
			}
			else
			{
				LOGW("Event time not enough info");
			}
		}
		else if (logical == 0 || logical == 1) // rule theo thiet bi dau vao, logical or
		{
			if (logical == 0)
			{
				type = "or";
			}
			else if (logical == 1)
			{
				type = "and";
			}
			rule = new Rule(id, type, repeat);
			if (!rule)
			{
				LOGW("New rule error");
			}
		}

		if (rule)
		{
			if (ruleValue.isMember("INPUT_DEVICES") && ruleValue["INPUT_DEVICES"].isArray())
			{
				Json::Value listDevInput = ruleValue["INPUT_DEVICES"];
				for (Json::ArrayIndex countDev = 0; countDev < listDevInput.size(); countDev++)
				{
					Json::Value devInput = listDevInput[countDev];
					if (devInput.isMember("DEVICE_ID") && devInput["DEVICE_ID"].isString() && devInput.isMember("DEVICE_ATTRIBUTE") && devInput["DEVICE_ATTRIBUTE"].isObject())
					{
						string devId = devInput["DEVICE_ID"].asString();
						Json::Value devAttribute = devInput["DEVICE_ATTRIBUTE"];
						Json::Value datasDevInput;
						if (devAttribute.isMember("ID") && devAttribute["ID"].isInt() && devAttribute.isMember("VALUE") && devAttribute["VALUE"].isArray())
						{
							int id = devAttribute["ID"].asInt();
							Json::Value values = devAttribute["VALUE"];
							string op = "<>";
							datasDevInput["ID"] = id;
							datasDevInput["VALUE"] = values;
							datasDevInput["OP"] = op;
						}
						Device *deviceInputRule = getDeviceFromId(devId);
						if (deviceInputRule)
						{
							RuleInputDevice *ruleInputDevice = new RuleInputDevice(rule, deviceInputRule, datasDevInput);
							if (ruleInputDevice)
							{
								rule->AddRuleInput(ruleInputDevice);
							}
							else
							{
								LOGW("create rule input device error");
							}
						}
						else
						{
							LOGW("Device %s does not exsit", devId.c_str());
						}
					}
				}
			}
			if (ruleValue.isMember("OUTPUT_DEVICES") && ruleValue["OUTPUT_DEVICES"].isArray())
			{
				Json::Value listDevOutput = ruleValue["OUTPUT_DEVICES"];
				for (Json::ArrayIndex countDevOp = 0; countDevOp < listDevOutput.size(); countDevOp++)
				{
					Json::Value devOutput = listDevOutput[countDevOp];
					if (devOutput.isMember("DEVICE_ID") && devOutput["DEVICE_ID"].isString() && devOutput.isMember("PROPERTIES") && devOutput["PROPERTIES"].isArray())
					{
						string devIdOp = devOutput["DEVICE_ID"].asString();
						Json::Value property = devOutput["PROPERTIES"];
						Device *deviceOutputRule = getDeviceFromId(devIdOp);
						if (deviceOutputRule)
						{
							RuleOutputDevice *ruleOutoutDevice = new RuleOutputDevice(deviceOutputRule, property);
							if (ruleOutoutDevice)
							{
								rule->AddRuleOutput(ruleOutoutDevice);
							}
							else
							{
								LOGW("Create rule output device error");
							}
						}
						else
						{
							LOGW("Device %s does not exsit", devIdOp.c_str());
						}
					}
				}
			}
			if (ruleValue.isMember("OUTPUT_GROUPS") && ruleValue["OUTPUT_GROUPS"].isArray())
			{
				Json::Value listGroupOutput = ruleValue["OUTPUT_GROUPS"];
				for (Json::ArrayIndex countGrp = 0; countGrp < listGroupOutput.size(); countGrp++)
				{
					Json::Value groupOutput = listGroupOutput[countGrp];
					if (groupOutput.isMember("GROUP_ID") && groupOutput["GROUP_ID"].isString() && groupOutput.isMember("PROPERTIES") && groupOutput["PROPERTIES"].isArray())
					{
						string groupId = groupOutput["GROUP_ID"].asString();
						Json::Value property = groupOutput["PROPERTIES"];
						Group *groupOutputRule = getGroupFromId(groupId);
						if (groupOutputRule)
						{
							RuleOutputGroup *ruleOutputGroup = new RuleOutputGroup(groupOutputRule, property);
							if (ruleOutputGroup)
							{
								rule->AddRuleOutput(ruleOutputGroup);
							}
							else
							{
								LOGW("Create rule output group error");
							}
						}
						else
						{
							LOGW("Group %s does not exsit", groupId.c_str());
						}
					}
				}
			}
			if (ruleValue.isMember("OUTPUT_SCENES") && ruleValue["OUTPUT_SCENES"].isArray())
			{
				Json::Value listScenesOutputRule = ruleValue["OUTPUT_SCENES"];
				for (Json::ArrayIndex countScene = 0; countScene < listScenesOutputRule.size(); countScene++)
				{
					Json::Value scenesOutputRule = listScenesOutputRule[countScene];
					if (scenesOutputRule.isObject())
					{
						if (scenesOutputRule.isMember("SCENE_ID") && scenesOutputRule["SCENE_ID"].isString())
						{
							string sceneId = scenesOutputRule["SCENE_ID"].asString();
							SceneBle *sceneOutputRule = getSceneBleFromId(sceneId);
							if (sceneOutputRule)
							{
								RuleOutputSceneBle *ruleOutputSceneBle = new RuleOutputSceneBle(sceneOutputRule);
								if (ruleOutputSceneBle)
								{
									rule->AddRuleOutput(ruleOutputSceneBle);
								}
								else
								{
									LOGW("Create rule output scene error");
								}
							}
							else
							{
								LOGW("Scene %s does not exsit", sceneId.c_str());
							}
						}
					}
				}
			}
			if (addGateway)
			{
				ruleList[id] = rule;
			}
			if (addDatabase)
			{
				string ruleStr = ruleValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->RuleAdd(id, ruleStr, status, 1);
			}
			bool isEnable = (status) ? true : false;
			rule->isEnable = isEnable;
		}
		return rule;
	}
	else
	{
		LOGW("Rule error format");
	}
	return NULL;
}

// Scene *Gateway::HandleRule

SceneBle *Gateway::AddNewSceneBle(SceneBle *sceneBle, bool addGateway, bool addDatabase)
{
	if (sceneBle)
	{
		if (addDatabase)
		{
			// int rs = database->GroupAdd(group);
			// if (rs)
			// {
			// LOGW("rs: %d", rs);
			// return NULL;
			// }
		}
		if (addGateway)
			sceneBleList[sceneBle->GetId()] = sceneBle;
	}
	return sceneBle;
}

Room *Gateway::AddNewRoom(Room *room)
{
	if (room)
	{
		roomList[room->GetUUId()] = room;
	}
	return room;
}

uint16_t Gateway::getBleUnicast()
{
	return ble_unicast;
}

string Gateway::getBleNetkey()
{
	return ble_netkey;
}
string Gateway::getBleAppKey()
{
	return ble_appkey;
}
string Gateway::getBleDeviceKey()
{
	return ble_devicekey;
}
string Gateway::getDormitory()
{
	return dormitoryId;
}
string Gateway::getId()
{
	return id;
}
string Gateway::getVersion()
{
	return version;
}
string Gateway::getName()
{
	return "";
}
string Gateway::getRefreshToken()
{
	return refresh_token;
}

void Gateway::setBleUnicast(uint16_t unicast)
{
	this->ble_unicast = unicast;
}
void Gateway::setBleNetkey(string netkey)
{
	this->ble_netkey = netkey;
}
void Gateway::setBleAppkey(string appkey)
{
	this->ble_appkey = appkey;
}
void Gateway::setBleDevicekey(string devicekey)
{
	this->ble_devicekey = devicekey;
}
void Gateway::setDormitory(string dormitory)
{
	this->dormitoryId = dormitory;
}
void Gateway::setRefreshToken(string refresh_token)
{
	this->refresh_token = refresh_token;
}
void Gateway::setId(string id)
{
	this->id = id;
}
void Gateway::setVersion(string version)
{
	this->version = version;
}
void Gateway::setName(string name)
{
}
