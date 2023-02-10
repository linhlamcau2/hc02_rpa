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

#include "RuleInputTimer.h"
#include "RuleOutputGroup.h"
#include "RuleOutputDevice.h"

#include "BleDefine.h"
#include "BleProtocol.h"
#include "DeviceBleLightOnoffCctDim.h"
#include "DeviceBleLightOnoffHslModeRGB.h"
#include "DeviceBleLightOnoffCctDimHslModeRGB.h"
#include "DeviceBleSwitchTouch4.h"
#include "DeviceBleSwitchScene6DC.h"
#include "DeviceBleSensorTempHum.h"
#include "DeviceBleSensorPm.h"

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

	cout << "scene size: " << sceneBleList.size() << endl;
	for (const auto &[meshId, scene] : sceneBleList)
	{
		cout << "scene: " + scene->GetUUId() << endl;
		for (Json::Value::ArrayIndex i = 0; i < scene->deviceList.size(); i++)
		{
			cout << "Device: " + scene->deviceList[i]->device->GetId() + scene->deviceList[i]->data.toString() << endl;
		}
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

	OnDeviceRPCCallbackRegister("SCAN", bind(&Gateway::OnRPCBleStartScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("STOP", bind(&Gateway::OnRPCBleStopScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("RESET_NODE", bind(&Gateway::OnRPCBleDelDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("RESET_BLE", bind(&Gateway::OnRPCBleResetFactory, this, placeholders::_1, placeholders::_2));

	OnDeviceRPCCallbackRegister("CREATE_GROUP", bind(&Gateway::OnRPCAddGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DELETE_GROUP", bind(&Gateway::OnRPCDelGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("ADD_DEVICE_TO_GROUP", bind(&Gateway::OnRPCAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("REMOVE_DEVICE_FROM_GROUP", bind(&Gateway::OnRPCDelDeviceFromGroup, this, placeholders::_1, placeholders::_2));

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

	OnLocalCallbackRegister("SCAN", bind(&Gateway::OnRPCBleStartScan, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("STOP", bind(&Gateway::OnRPCBleStopScan, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("RESET_NODE", bind(&Gateway::OnRPCBleDelDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("RESET_BLE", bind(&Gateway::OnRPCBleResetFactory, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_GROUP", bind(&Gateway::OnRPCAddGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_GROUP", bind(&Gateway::OnRPCDelGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("ADD_DEVICE_TO_GROUP", bind(&Gateway::OnRPCAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("REMOVE_DEVICE_FROM_GROUP", bind(&Gateway::OnRPCDelDeviceFromGroup, this, placeholders::_1, placeholders::_2));

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

void Gateway::resetFactory()
{
	LOGI("resetFactory");
	database->DeviceDelAll();
	database->GatewayDelAll();
	database->DeviceAttributeDelAll();
	database->GroupDelAll();
	database->DeviceInGroupDelAll();
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
	propertyValue["VALUE"] = 0;
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
	for (int i = 0; i < 60; i++)
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

int Gateway::OnRPCBleResetFactory(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("Reset ble");
	bleProtocol->ResetFactory();
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
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			string deviceId = dataValue[i].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				bleProtocol->ResetDev(device->GetAddr());
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
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		Rule *rule = AddRule(dataValue, true, true);
		if (rule)
		{
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCDeleteRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDeleteRule");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("id") && dataValue["id"].isInt())
		{
			int ruleId = dataValue["id"].asInt();
			LOGI("Delete Rule id: %d", ruleId);
			delete ruleList[ruleId];
			ruleList.erase(ruleList.find(ruleId));
			database->RuleDel(ruleId);
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCAddSceneBle(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddSceneBle");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString() &&
				dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
		{
			string sceneId = dataValue["SCENE_ID"].asString();
			int temp_sceneUnicastId = 1;
			for (auto &x : sceneBleList)
			{
				if (x.first >= temp_sceneUnicastId)
				{
					temp_sceneUnicastId = x.first + 1;
				}
			}
			cout << "scene id: " << temp_sceneUnicastId << endl;
			SceneBle *scene = new SceneBle(sceneId, temp_sceneUnicastId, sceneId);
			if (scene)
			{
				scene = AddNewSceneBle(scene, true, true);
				if (scene)
				{
					Json::Value groupList = dataValue["DEVICES"];
					for (Json::ArrayIndex i = 0; i < groupList.size(); i++)
					{
						Json::Value deviceList = groupList[i]["IDS"];
						Json::Value deviceProperties = groupList[i]["PROPERTIES"];
						for (Json::ArrayIndex j = 0; j < deviceList.size(); j++)
						{
							string devcieId = deviceList[j].asString();
							Device *device = getDeviceFromId(devcieId);
							if (device)
							{
								int tempDeviceAddr = device->GetAddr();
								if (scene->AddDevice(device, deviceProperties, tempDeviceAddr, false))
								{
									database->DeviceInSceneBleAdd(scene, device, deviceProperties);
								}
							}
						}
					}
				}
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCEditSceneBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("SCENE_ID") && dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
		{
			string sceneId = dataValue["SCENE_ID"].asString();
			SceneBle *scene = getSceneBleFromId(sceneId);
			if (scene)
			{
				Json::Value groupList = dataValue["DEVICES"];
				for (Json::ArrayIndex i = 0; i < groupList.size(); i++)
				{
					Json::Value deviceList = groupList[i]["IDS"];
					Json::Value deviceProperties = groupList[i]["PROPERTIES"];
					for (Json::ArrayIndex j = 0; j < deviceList.size(); j++)
					{
						string devcieId = deviceList[j].asString();
						Device *device = getDeviceFromId(devcieId);
						if (device)
						{
							int tempDeviceAddr = device->GetAddr();
							if (scene->AddDevice(device, deviceProperties, tempDeviceAddr, false))
							{
								database->DeviceInSceneBleAdd(scene, device, deviceProperties);
								// respValue["code"] = 0;
								// return 0;
							}
						}
					}
				}
			}
		}
	}
	// respValue["code"] = -1;
	return true;
}

int Gateway::OnRPCDeleteSceneBle(Json::Value &reqValue, Json::Value &respValue)
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
				for (unsigned int i = 0; i < scene->deviceList.size(); i++)
				{
					scene->DelDevice(scene->deviceList[i]->device);
				}
				int temp_sceneUnicastId = scene->GetId();
				delete sceneBleList[temp_sceneUnicastId];
				database->SceneBleDel(scene);
				respValue["code"] = 0;
				return 0;
			}
		}
	}
	respValue["code"] = -1;
	return -1;
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
						Json::Value deviceList = dataValue["DEVICES"];
						for (Json::ArrayIndex i = 0; i < deviceList.size(); i++)
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
				for (unsigned int i = 0; i < group->deviceList.size(); i++)
				{
					if (group->DelDevice(group->deviceList[i]->device, group->deviceList[i]->device->GetAddr()))
					{
						database->DeviceInGroupAdd(group, group->deviceList[i]->device, temp_groupUnicastId);
						data["SUCCESS"].append(group->deviceList[i]->device->GetId());
					}
					else
					{
						hasDeviceDelGroupFailed = true;
						data["FAILED"].append(group->deviceList[i]->device->GetId());
					}
				}
				if (hasDeviceDelGroupFailed)
				{
					database->GroupDel(group->GetId());
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
			respValue["CMD"] = "REMOVE_DEVICE_FROM_GROUP";
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
					device->DoJsonArray(properties);
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
		}
	}
	return 0;
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
	case BLE_SWITCH_4:
		device = new DeviceBleSwitchTouch4(id, name, mac, device_id, addr, version);
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
	// TODO: Check Rule id exist
	if (ruleValue.isMember("EVENT_TRIGGER_ID") && ruleValue["EVENT_TRIGGER_ID"].isString() &&
			ruleValue.isMember("PRIORITY") && ruleValue["PRIORITY"].isInt() &&
			ruleValue.isMember("START_AT") && ruleValue["START_AT"].isString() &&
			ruleValue.isMember("END_AT") && ruleValue["END_AT"].isString() &&
			ruleValue.isMember("TURN_OFF_AT") && ruleValue["TURN_OFF_AT"].isString() &&
			ruleValue.isMember("FADE_IN") && ruleValue["FADE_IN"].isInt() &&
			ruleValue.isMember("FADE_OUT") && ruleValue["FADE_OUT"].isInt() &&
			ruleValue.isMember("EACH_DAY") && ruleValue["EACH_DAY"].isArray() &&
			ruleValue.isMember("LOGICAL_OPERATOR_ID") && ruleValue["LOGICAL_OPERATOR_ID"].isInt() &&
			ruleValue.isMember("STATUS") && ruleValue["STATUS"].isInt() &&
			ruleValue.isMember("INPUT_DEVICES") && ruleValue["INPUT_DEVICES"].isArray() &&
			ruleValue.isMember("OUTPUT_DEVICES") && ruleValue["OUTPUT_DEVICES"].isArray() &&
			ruleValue.isMember("OUTPUT_GROUPS") && ruleValue["OUTPUT_GROUPS"].isArray() &&
			ruleValue.isMember("OUTPUT_SCENES") && ruleValue["OUTPUT_SCENES"].isArray())
	{
		string id = ruleValue["EVENT_TRIGGER_ID"].asString();
		// int repeat = ruleValue["EACH_DAY"].asInt();
		Json::Value repeatDays = ruleValue["EACH_DAY"];
		int mon = 0, tue = 0, wed = 0, thu = 0, fri = 0, sat = 0, sun = 0;
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
		int repeat = Util::ConvertRepeatDayToInt(mon, tue, wed, thu, fri, sat, sun);
		bool fullDay = (ruleValue["START_AT"].asString() == "0:0:0" && ruleValue["END_AT"].asString() == "23:59:59") ? true : false;
		// string type = ruleValue["LOGICAL_OPERATOR_ID"].asInt() == 0 ? "and" : "or";
		Rule *rule = NULL;

		// TODO: Check Type of Rule:
		/*
			- -1: Rule Time
			- +0: Rule OR
			- +1: Rule AND
			- +2: Rule Time + OR
			- +3: Rule Time + AND
		*/

		int ruleLogicId = ruleValue["LOGICAL_OPERATOR_ID"].asInt();
		if (ruleLogicId == -1)
		{
			string type = "and";
			if (!fullDay && ruleValue.isMember("START_AT") && ruleValue["START_AT"].isString() &&
					ruleValue.isMember("END_AT") && ruleValue["END_AT"].isString())
			{
				string startTime = ruleValue["START_AT"].asString();
				string endTime = ruleValue["END_AT"].asString();
				rule = new Rule(id, type, repeat, Util::ConvertStrTimeToInt(startTime), Util::ConvertStrTimeToInt(endTime));
			}
			else
			{
				rule = new Rule(id, type, repeat);
			}
			if (!rule)
			{
				LOGE("New rule error");
				return NULL;
			}
		}
		if (!rule)
		{
			string type = "or";
			rule = new Rule(id, type, repeat);
			if (!rule)
			{
				LOGE("New rule error");
				return NULL;
			}
		}
		Json::Value inputValue = ruleValue["input"];
		if (inputValue.isMember("timer") && inputValue["timer"].isObject())
		{
			string type = "and";
			rule = new Rule(id, type, repeat);
			if (!rule)
			{
				LOGE("New rule error");
				return NULL;
			}
		}
		else if (ruleLogicId == 2)
		{
			string type = "or";
			string startTime = ruleValue["START_AT"].asString();
			string endTime = ruleValue["END_AT"].asString();
			rule = new Rule(id, type, repeat, Util::ConvertStrTimeToInt(startTime), Util::ConvertStrTimeToInt(endTime));
			if (!rule)
			{
				LOGE("New rule error");
				return NULL;
			}
		}
		else if (ruleLogicId == 3)
		{
			string type = "and";
			string startTime = ruleValue["START_AT"].asString();
			string endTime = ruleValue["END_AT"].asString();
			rule = new Rule(id, type, repeat, Util::ConvertStrTimeToInt(startTime), Util::ConvertStrTimeToInt(endTime));
			if (!rule)
			{
				LOGE("New rule error");
				return NULL;
			}
		}
		// Handle output device
		Json::Value deviceRuleOutputList = ruleValue["OUTPUT_DEVICES"];
		for (Json::ArrayIndex i = 0; i < deviceRuleOutputList.size(); i++)
		{
			Json::Value deviceRuleOutputValue = deviceRuleOutputList[i];
			if (deviceRuleOutputValue.isObject())
			{
				if (deviceRuleOutputValue.isMember("DEVICE_ID") && deviceRuleOutputValue["DEVICE_ID"].isString() &&
						deviceRuleOutputValue.isMember("PROPERTIES") && deviceRuleOutputValue["PROPERTIES"].isObject())
				{
					Json::Value dataValue = deviceRuleOutputValue["PROPERTIES"];
					string deviceId = deviceRuleOutputValue["DEVICE_ID"].asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						RuleOutputDevice *ruleOutputDevice = new RuleOutputDevice(device, dataValue);
						rule->AddRuleOutput(ruleOutputDevice);
					}
				}
			}
		}
		// Handle output group
		Json::Value groupRuleOutputList = ruleValue["OUTPUT_GROUPS"];
		for (Json::ArrayIndex i = 0; i < groupRuleOutputList.size(); i++)
		{
			Json::Value groupRuleOutputValue = groupRuleOutputList[i];
			if (groupRuleOutputValue.isObject())
			{
				if (groupRuleOutputValue.isMember("GROUP_ID") && groupRuleOutputValue["GROUP_ID"].isString() &&
						groupRuleOutputValue.isMember("PROPERTIES") && groupRuleOutputValue["PROPERTIES"].isObject())
				{
					string id = groupRuleOutputValue["GROUP_ID"].asString();
					Json::Value dataValue = groupRuleOutputValue["PROPERTIES"];
					Group *group = getGroupFromId(id);
					if (group)
					{
						RuleOutputGroup *ruleOutputGroup = new RuleOutputGroup(group, dataValue);
						rule->AddRuleOutput(ruleOutputGroup);
					}
				}
			}
		}
		// Handle output scene
		Json::Value sceneRuleOutputList = ruleValue["OUTPUT_SCENES"];
		for (Json::ArrayIndex i = 0; i < groupRuleOutputList.size(); i++)
		{
			string sceneRuleOutputId = sceneRuleOutputList[i].asString();
			SceneBle *sceneBle = getSceneBleFromId(sceneRuleOutputId);
			if (sceneBle)
			{
				RuleOutputSceneBle *ruleOutputSceneBle = new RuleOutputSceneBle(sceneBle);
				rule->AddRuleOutput(ruleOutputSceneBle);
			}
		}
		// LOGI("Add Rule %s", rule->GetId().c_str());
		// if (addGateway)
		// 	ruleList[rule->GetId()] = rule;
		if (addDatabase)
		{
			string ruleStr = ruleValue.toString();
			ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
			// database->RuleAdd(rule->GetId(), ruleStr);
		}
		rule->Check();
		return rule;
	}
	else
	{
		LOGW("Rule format error");
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
