#include "Gateway.h"
#include "Log.h"
#include <unistd.h>
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <fstream>
#include <iostream>
#include <thread>
#include "json.h"
#include "Db.h"
#include "Util.h"
#include "Wifi.h"
#include "Ota.h"
#include "Base64.h"
#include "Config.h"
#include "DeviceBleAll.h"
#include "DeviceBleSwitchOnoff.h"
#include "DeviceBleLightOnoffCctDim.h"
#include "DeviceBleLightOnoffHslModeRGB.h"
#include "DeviceBleLightOnoffCctDimHslModeRGB.h"
#include "DeviceBleSwitchTouchRgb.h"
#include "DeviceBleSwitchElectrical.h"
#include "DeviceBleSwitchScene6DC.h"
#include "DeviceBleSwitchScene6AC.h"
#include "DeviceBleSwitchScene6ACRgb.h"
#include "DeviceBleSensorTempHum.h"
#include "DeviceBleSensorPm.h"
#include "DeviceBlePirLightSensorDC.h"
#include "DeviceBlePirLightSensorAC.h"
#include "DeviceBleSmokeSensor.h"
#include "DeviceBleDoorSensor.h"
#include "DeviceBleScreenTouch.h"
#include "DeviceBleCurtain.h"
#include "DeviceBleRoolDoor.h"
#include "DeviceBleSwitchTouch.h"

#ifdef ESP_PLATFORM
#include "Config.h"
#include "Led.h"
#include "esp_spiffs.h"
#else
#include "DeviceMqttAihub.h"
#endif
#include "Http.h"

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#include "DeviceZigbeeOnoff.h"
#include "DeviceZigbeeTelinkOnoff.h"
#endif

Gateway *gateway = NULL;

Gateway::Gateway(string mac, string address, int port, string clientId, string username, string password, int keepalive, string localAddress, int localPort, string localUsername, string localPassword, int localKeepalive)
		: CloudProtocol(mac, address, port, clientId, username, password, keepalive),
			LocalProtocol(mac, localAddress, localPort, mac, localUsername, localPassword, localKeepalive),
			Udp(8181)
{
	this->mac = mac;
	this->id = "";
	this->dormitoryId = "";
	this->refresh_token = "";
	this->ble_addr = 0;
	this->ble_iv_index = 0;
	this->ble_appkey = "";
	this->ble_appkey = "";
	this->ble_devicekey = "";
	this->numScreenTouchs = 0;
}

Gateway::~Gateway()
{
}

Device *Gateway::getDeviceFromMac(string mac)
{
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		if (device->GetMac() == mac)
		{
			deviceListMtx.unlock();
			return device;
		}
	}
	deviceListMtx.unlock();
	return NULL;
}

Device *Gateway::getDeviceFromId(string id)
{
	deviceListMtx.lock();
	for (const auto &[idDev, device] : deviceList)
	{
		if (device->CheckId(id))
		{
			deviceListMtx.unlock();
			return device;
		}
	}
	// if (deviceList.find(id) != deviceList.end())
	// {
	// 	deviceListMtx.unlock();
	// 	return deviceList[id];
	// }
	deviceListMtx.unlock();
	return NULL;
}

DeviceBle *Gateway::getDeviceBleFromAddr(uint32_t addr)
{
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		if (device->CheckAddr(addr) && device->GetProtocol() == BLE_DEVICE)
		{
			DeviceBle *deviceBle = dynamic_cast<DeviceBle *>(device);
			if (deviceBle)
			{
				deviceListMtx.unlock();
				return deviceBle;
			}
		}
	}
	deviceListMtx.unlock();
	return NULL;
}

void Gateway::delDevice(Device *device)
{
	deviceListMtx.lock();
	deviceList.erase(device->GetId());
	deviceListMtx.unlock();
	database->DeviceDel(device);
	delete device;
}

Group *Gateway::getGroupFromId(string id)
{
	groupListMtx.lock();
	if (groupList.find(id) != groupList.end())
	{
		groupListMtx.unlock();
		return groupList[id];
	}
	groupListMtx.unlock();
	return NULL;
}

Group *Gateway::getGroupFromAddr(int addr)
{
	groupListMtx.lock();
	for (const auto &[id, group] : groupList)
	{
		if (group->GetAddr() == addr)
		{
			groupListMtx.unlock();
			return group;
		}
	}
	groupListMtx.unlock();
	return NULL;
}

void Gateway::delGroup(Group *group)
{
	groupListMtx.lock();
	groupList.erase(group->GetId());
	groupListMtx.unlock();
	database->GroupDel(group);
	delete group;
}

uint32_t Gateway::getNextGroupAddr()
{
	uint32_t groupAddr = 0;
	groupListMtx.lock();
	for (const auto &[id, group] : groupList)
	{
		if (group->GetAddr() >= groupAddr)
		{
			groupAddr = group->GetAddr() + 1;
		}
	}
	groupListMtx.unlock();
	return groupAddr;
}

SceneBle *Gateway::getSceneBleFromId(string id)
{
	sceneBleListMtx.lock();
	if (sceneBleList.find(id) != sceneBleList.end())
	{
		sceneBleListMtx.unlock();
		return sceneBleList[id];
	}
	sceneBleListMtx.unlock();
	return NULL;
}

SceneBle *Gateway::getSceneBleFromAddr(int addr)
{
	sceneBleListMtx.lock();
	for (const auto &[id, sceneBle] : sceneBleList)
	{
		if (sceneBle->GetAddr() == addr)
		{
			sceneBleListMtx.unlock();
			return sceneBle;
		}
	}
	sceneBleListMtx.unlock();
	return NULL;
}

void Gateway::delSceneBle(SceneBle *sceneBle)
{
	sceneBleListMtx.lock();
	sceneBleList.erase(sceneBle->GetId());
	sceneBleListMtx.unlock();
	database->SceneBleDel(sceneBle);
	delete sceneBle;
}

uint32_t Gateway::getNextSceneBleAddr()
{
	uint32_t sceneAddr = 1;
	sceneBleListMtx.lock();
	for (const auto &[id, sceneBle] : sceneBleList)
	{
		if (sceneBle->GetAddr() >= sceneAddr)
		{
			sceneAddr = sceneBle->GetAddr() + 1;
		}
	}
	sceneBleListMtx.unlock();
	return sceneAddr;
}

Rule *Gateway::getRuleFromId(string id)
{
	ruleListMtx.lock();
	if (ruleList.find(id) != ruleList.end())
	{
		ruleListMtx.unlock();
		return ruleList[id];
	}
	ruleListMtx.unlock();
	return NULL;
}

void Gateway::delRule(Rule *rule)
{
	ruleListMtx.lock();
	ruleList.erase(rule->GetId());
	ruleListMtx.unlock();
	database->RuleDel(rule);
	delete rule;
}

Room *Gateway::getRoomFromId(string id)
{
	roomListMtx.lock();
	if (roomList.find(id) != roomList.end())
	{
		roomListMtx.unlock();
		return roomList[id];
	}
	roomListMtx.unlock();
	return NULL;
}

void Gateway::delRoom(Room *room)
{
	roomListMtx.lock();
	roomList.erase(room->GetId());
	roomListMtx.unlock();
	database->RoomDel(room);
	delete room;
}

#ifdef ESP_PLATFORM
static void startCheckOnlineThread(void *data)
{
	Gateway *gateway = (Gateway *)data;
	gateway->CheckOnlineThread();
	vTaskDelete(NULL);
}

static void startUdpThread(void *data)
{
	Gateway *gateway = (Gateway *)data;
	gateway->UdpBroadcastThread();
	vTaskDelete(NULL);
}
#endif

void Gateway::init()
{
	CloudProtocol::init();
	LocalProtocol::init();
	Udp::init();

	initUdpMessage();
	initMqttMessage();

#ifdef ESP_PLATFORM
	LOGI("Free memory: %d bytes, internal: %d bytes", esp_get_free_heap_size(), esp_get_free_internal_heap_size());
	if (xTaskCreate(startUdpThread, "Udp", 5120, this, 7, NULL) != pdPASS)
	{
		LOGE("Failed to create task");
		SetLedService(false);
	}
	vTaskDelay(10);
	if (xTaskCreate(startCheckOnlineThread, "CheckOnline", 5120, this, 7, NULL) != pdPASS)
	{
		LOGE("Failed to create task");
		SetLedService(false);
	}
	vTaskDelay(10);
#else
	thread udpBroadcastThread(bind(&Gateway::UdpBroadcastThread, this));
	udpBroadcastThread.detach();
	thread checkOnlineThread(bind(&Gateway::CheckOnlineThread, this));
	checkOnlineThread.detach();
#endif

	database->GatewayRead();
	database->DeviceRead();
	gateway->AddNewDevice("", "all", "ble", "eyJkZXZpY2VrZXkiOiIifQ==", 65535, 0, 0, true, false);
	database->DeviceBleChildRead();
	database->DeviceAttributeRead();
	database->RoomRead();
	database->GroupRead();
	database->DeviceInGroupRead();
	database->SceneBleRead();
	database->DeviceInSceneBleRead();
	database->DeviceInRoomRead();
	database->RuleRead();
	if (gateway->getId().compare("") == 0)
	{
		id = mac;
		gateway->setId(id);
		database->GatewayAdd(gateway);
		database->GatewayRead();
	}

	CloudConnect();
	LocalConnect();
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
			deviceListMtx.lock();
			for (const auto &[id, device] : deviceList)
			{
				device->PushAttributes();
			}
			deviceListMtx.unlock();
		}
#ifdef ESP_PLATFORM
		SetLedInternet(true);
#endif
	}
	else
	{
		Util::LedInternet(false);
#ifdef ESP_PLATFORM
		if (GetModeLedInternet() != LED_BLINK && GetModeLedInternet() != LED_FLASH)
			SetLedInternet(false);
#endif
	}
}

void Gateway::OnLocalConnect(bool isConnected, bool isReconnect)
{
	LOGI("OnLocalConnect: %d", isConnected);
}

void Gateway::DelDatabase()
{
	delete database;
#ifdef ESP_PLATFORM
	if (unlink(DB_NAME) != 0)
	{
		LOGE("Failed to delete file\n");
	}

	// Unmount SPIFFS
	esp_vfs_spiffs_unregister(NULL);
#else
	string rmDb = "rm " DB_NAME;
	system(rmDb.c_str());
#endif
}

void Gateway::ResetFactory()
{
	LOGI("ResetFactory");
	if (bleProtocol)
	{
		bleProtocol->ResetDelAll();
		bleProtocol->ResetFactory();
	}
	else
		LOGW("BleProtocol null");

	DelDatabase();
}

void Gateway::SendDataForScreenTouch(Device *device, string &dataWeather, uint8_t statusWeather, uint16_t temp)
{
	if (bleProtocol)
	{
		bleProtocol->SendDate(device->GetAddr(), Util::GetYearsCurrent(), Util::GetMonthsCurrent(), Util::GetDateCurrent(), Util::GetDaysCurrent());
		bleProtocol->SendTime(device->GetAddr(), Util::GetHoursCurrent(), Util::GetMinutesCurrent(), Util::GetSecondsCurrent());
		bleProtocol->SendWeatherIndoor(device->GetAddr(), Util::GetTempOfScreenTouch() / 10, Util::GetHumOfScreenTouch() / 10, 0);
		if (statusWeather != 254 && temp != 65534)
			bleProtocol->SendWeatherOutdoor(device->GetAddr(), statusWeather, temp);
	}
	else
		LOGW("BleProtocol null");
}

static string ST_array_icon[18] = {"01d", "02d", "03d", "04d", "09d", "10d", "11d", "13d", "50d", "01n", "02n", "03n", "04n", "09n", "10n", "11n", "13n", "50n"};

int Gateway::CheckOnlineThread()
{
	LOGI("Start CheckOnlineThread");
	time_t currentTime = 0;
	time_t oldTime = 0;
	uint32_t allTimeCheck = 0; // time total in a loop check
	bool deviceStateChange = false;

	Json::Value devicesData;
	Json::Value onlineValue;
	Json::Value offlineValue;
	offlineValue["stt"] = 0;
	onlineValue["stt"] = 1;

	while (!bleProtocol)
	{
		sleep(1);
	}

	while (1)
	{
		// Check have device screen touch -> send datetime, weather data
		if (numScreenTouchs > 0 && (time(NULL) - oldTime) > 1800)
		{
			oldTime = time(NULL);
			HTTPRequest *httpRequest = new HTTPRequest();
			string dataWeather = httpRequest->GetWeather(Util::GetLongitude(), Util::GetLatitude());
			delete httpRequest;
			LOGI("dataWeather:%s", dataWeather.c_str());

			uint8_t status = 254;
			uint16_t temp = 65534;

			Json::Value dataWeatherJson;
			if (dataWeatherJson.parse(dataWeather) && dataWeatherJson.isObject())
			{
				if (dataWeatherJson.isMember("weather") && dataWeatherJson["weather"].isArray() &&
						dataWeatherJson.isMember("main") && dataWeatherJson["main"].isObject())
				{
					Json::Value weather = dataWeatherJson["weather"][0];
					Json::Value main = dataWeatherJson["main"];
					if (weather.isMember("icon") && weather["icon"].isString() && main.isMember("temp") && main["temp"].isDouble())
					{
						string icon = weather["icon"].asString();
						for (int i = 0; i < 17; i++)
						{
							if (icon.compare(ST_array_icon[i]) == 0)
							{
								status = i;
								break;
							}
						}
						temp = main["temp"].asInt();
					}
				}
			}

			deviceListMtx.lock();
			for (const auto &[id, device] : deviceList)
			{
				if (device->GetType() == BLE_AC_SCENE_SCREEN_TOUCH)
				{
					SendDataForScreenTouch(device, dataWeather, status, temp);
				}
			}
			deviceListMtx.unlock();
		}

		if (!bleProtocol->IsProvision() && !LocalProtocol::IsBusy() && !CloudProtocol::IsBusy())
		{
			// deviceListMtx.lock();
			devicesData = Json::Value::null;
			allTimeCheck = deviceList.size() * 4;
			for (const auto &[id, device] : deviceList)
			{
				if (!bleProtocol->IsProvision() && !LocalProtocol::IsBusy() && !CloudProtocol::IsBusy())
				{
					if (device->GetAddr() != 65535)
					{
						currentTime = time(NULL);
						deviceStateChange = false;
						if (device->lastOnlineState) // online
						{
							// neu thiet bi ho tro ban tin check trang thai online/offline
							if (device->isNeedCheckOnline())
							{
								// thoi gian lan cuoi cung nhan ban tin hoac lan cuoi cung check qua 1 chu ky
								if ((device->lastTimeActive + allTimeCheck) <= currentTime && (device->lastTimeCheckActive + allTimeCheck) <= currentTime)
								{
									bleProtocol->SendOnlineCheck(device->GetAddr(), device->GetType(), device->GetVersion());
									device->lastTimeCheckActive = currentTime;
								}
								// 2 chu ky khong co ban tin phan hoi thi bao offline
								if ((device->lastTimeActive + allTimeCheck * 2 + 1) < currentTime)
								{
									LOGI("Device %s addr 0x%04X offline", device->GetName().c_str(), device->GetAddr());
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
									LOGI("Device %s addr 0x%04X offline", device->GetName().c_str(), device->GetAddr());
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
								if ((device->lastTimeCheckActive + allTimeCheck) <= currentTime)
								{
									bleProtocol->SendOnlineCheck(device->GetAddr(), device->GetType(), device->GetVersion());
									device->lastTimeCheckActive = currentTime;
								}
								// neu co ban tin moi trong vong 2 chu ky check thi bao online
								if ((device->lastTimeActive + allTimeCheck * 2) >= currentTime)
								{
									LOGI("Device %s addr 0x%04X online", device->GetName().c_str(), device->GetAddr());
									device->lastOnlineState = true;
									deviceStateChange = true;
								}
							}
							else
							{
								// trong ngay co ban tin thi online
								if ((device->lastTimeActive + 60 * 60 * 24) >= currentTime)
								{
									LOGI("Device %s addr 0x%04X online", device->GetName().c_str(), device->GetAddr());
									device->lastOnlineState = true;
									deviceStateChange = true;
								}
							}
						}
						// send device state to server
						if (deviceStateChange)
						{
							Json::Value deviceData;
							deviceData["id"] = device->GetId();
							if (device->lastOnlineState)
								deviceData["data"] = onlineValue;
							else
								deviceData["data"] = offlineValue;
							devicesData.append(deviceData);
						}
					}
				}
				else
					break;
			}
			// deviceListMtx.unlock();
			if (!devicesData.isNull())
			{
				Json::Value dataValue;
				dataValue["device"] = devicesData;
				gateway->pushDeviceUpdateLocal(dataValue);
				gateway->pushDeviceUpdateCloud(dataValue);
			}
		}
		sleep(1);
	}
	return CODE_OK;
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
#ifdef ESP_PLATFORM
	hcBroadcastValue["NAME"] = "minihub" + mac;
#else
	hcBroadcastValue["NAME"] = "hc" + mac;
#endif
	hcBroadcastValue["REQUEST_ID"] = Util::genRandRQI(16);
	hcBroadcastValue["TIME"] = Util::GetCurrentTimeStr();
	hcBroadcastValue["CONNECTION_TYPE"] = 0;
	hcInfoValue["TYPE"] = 3;
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
	string dataStr = hcBroadcastValue.toString();
	bool ledInternet = Util::GetStatusLedInternet();
	isUdpBroadcasting = false;
	while (1)
	{
		if (isUdpBroadcasting)
		{
			Util::LedInternet(false);
			usleep(500000);
			send(dataStr, &s, sizeof(s));
			Util::LedInternet(true);
			usleep(500000);
		}
		else
		{
			sleep(1);
		}
	}
	return CODE_OK;
}

void Gateway::StartUdpBroadcast()
{
	LOGW("StartUdpBroadcast");
	isUdpBroadcasting = true;
}

void Gateway::StopUdpBroadcast()
{
	LOGW("StopUdpBroadcast");
	isUdpBroadcasting = false;
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
	respValue["CONNECTION_TYPE"] = 3; // Connect mqtt to cloud

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

static Json::Value PushNewDevMqtt(Device *newDev)
{
	Json::Value jsonValue;
	Json::Value devValue;
	devValue["id"] = newDev->GetId();
	devValue["addr"] = newDev->GetAddr();
	devValue["type"] = newDev->GetType();
	devValue["ver"] = newDev->GetVersionStr();
	devValue["mac"] = newDev->GetMac();
	string data = newDev->GetData();
	string devKey = "";
	Json::Value json;
	json.parse(data);
	devValue["data"] = json;
	jsonValue["device"].append(devValue);
	return jsonValue;
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
	Device *temp_dev = getDeviceFromId(scanDevice->GetId());
	if (temp_dev)
	{
		database->DelDevExist(temp_dev);
	}
	string data = scanDevice->GetData();
	string devKey = "";
	Json::Value json;
	if (json.parse(data) && json.isObject())
	{
		if (json.isMember("devicekey") && json["devicekey"].isString())
		{
			devKey = json["devicekey"].asString();
		}
	}

	dataValue["DEVICE_ID"] = scanDevice->GetId();
	dataValue["DEVICE_UNICAST_ID"] = (int)scanDevice->GetAddr();
	dataValue["DEVICE_TYPE_ID"] = (int)scanDevice->GetType();
	dataValue["MAC_ADDRESS"] = scanDevice->GetMac();
	dataValue["FIRMWARE_VERSION"] = scanDevice->GetVersionStr();
	dataValue["DEVICE_KEY"] = devKey;
	dataValue["NET_KEY"] = gateway->getBleNetKey();
	dataValue["APP_KEY"] = gateway->getBleAppKey();
	jsonValue["CMD"] = "NEW_DEVICE";
	jsonValue["DATA"] = dataValue;
#ifndef CONFIG_USE_MESSAGE_FORMAT_V2
	LocalPublish(jsonValue);
#else
	CloudPublish(jsonValue);
#endif

#ifndef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue["CMD"] = "NEW_CHILD_DEVICE";
	if (scanDevice->GetType() == BLE_SWITCH_RGB_2 || scanDevice->GetType() == BLE_SWITCH_RGB_2_SQUARE || scanDevice->GetType() == BLE_SWITCH_ELECTRICAL_2)
	{
		dataValue["PARENT_DEVICE_ID"] = scanDevice->GetId();
		dataValue["DEVICE_ID"] = Util::GenIdDeviceByElement(scanDevice->GetId(), 1);
		dataValue["DEVICE_UNICAST_ID"] = (int)scanDevice->GetAddr() + 1;
		dataValue["BUTTON_ID"] = 12;
		jsonValue["DATA"] = dataValue;
		LocalPublish(jsonValue);
	}
	else if (scanDevice->GetType() == BLE_SWITCH_RGB_3 || scanDevice->GetType() == BLE_SWITCH_RGB_3_SQUARE || scanDevice->GetType() == BLE_SWITCH_ELECTRICAL_3)
	{
		for (int i = 1; i <= 2; i++)
		{
			dataValue["PARENT_DEVICE_ID"] = scanDevice->GetId();
			dataValue["DEVICE_ID"] = Util::GenIdDeviceByElement(scanDevice->GetId(), i);
			dataValue["DEVICE_UNICAST_ID"] = (int)scanDevice->GetAddr() + i;
			dataValue["BUTTON_ID"] = 11 + i;
			jsonValue["DATA"] = dataValue;
			LocalPublish(jsonValue);
		}
	}
	else if (scanDevice->GetType() == BLE_SWITCH_RGB_4 || scanDevice->GetType() == BLE_SWITCH_RGB_4_SQUARE || scanDevice->GetType() == BLE_SWITCH_ELECTRICAL_4)
	{
		for (int i = 1; i <= 3; i++)
		{
			dataValue["PARENT_DEVICE_ID"] = scanDevice->GetId();
			dataValue["DEVICE_ID"] = Util::GenIdDeviceByElement(scanDevice->GetId(), i);
			dataValue["DEVICE_UNICAST_ID"] = (int)scanDevice->GetAddr() + i;
			dataValue["BUTTON_ID"] = 11 + i;
			jsonValue["DATA"] = dataValue;
			LocalPublish(jsonValue);
		}
	}
	else if (scanDevice->GetType() == BLE_AC_SCENE_CONTACT_RGB || scanDevice->GetType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE)
	{
		for (int i = 1; i <= 5; i++)
		{
			dataValue["PARENT_DEVICE_ID"] = scanDevice->GetId();
			dataValue["DEVICE_ID"] = Util::GenIdDeviceByElement(scanDevice->GetId(), i);
			dataValue["DEVICE_UNICAST_ID"] = (int)scanDevice->GetAddr();
			dataValue["BUTTON_ID"] = 11 + i;
			jsonValue["DATA"] = dataValue;
			LocalPublish(jsonValue);
		}
	}
#endif
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	Json::Value jsonData = PushNewDevMqtt(scanDevice);
	pushNewDeviceLocal(jsonData);
#endif
}

Device *Gateway::AddNewDevice(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool addGateway, bool addDatabase)
{
	LOGI("Add new device id: %s, name: %s, mac: %s, addr: 0x%04X, type: 0x%04X, verion: %d", id.c_str(), name.c_str(), mac.c_str(), addr, type, version);
	Device *device = NULL;
	switch (type)
	{
	case BLE_ALL:
		device = new DeviceBleAll(id, name, mac, data, addr, type, version);
		break;
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
		device = new DeviceBleLightOnoffCctDim(id, name, mac, data, addr, type, version);
		break;
	case BLE_DOWNLIGHT_RGBCW:
	case BLE_LED_DAY_RGBCW:
	case BLE_LED_BULB:
		device = new DeviceBleLightOnoffCctDimHslModeRGB(id, name, mac, data, addr, type, version);
		break;
	case BLE_LED_DAY_RGB:
		device = new DeviceBleLightOnoffHslModeRGB(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_ONOFF:
		device = new DeviceBleSwitchOnoff(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_RGB_1:
	case BLE_SWITCH_RGB_1_SQUARE:
	case BLE_SWITCH_RGB_WATER_HEATER:
		device = new DeviceBleSwitchTouchRgb(id, name, mac, data, addr, type, version, 1);
		break;
	case BLE_SWITCH_RGB_2:
	case BLE_SWITCH_RGB_2_SQUARE:
		for (int i = 1; i < 2; i++)
		{
			device = new DeviceBleSwitchTouchRgb(Util::GenIdDeviceByElement(id, i), name, mac, data, addr + i, type, version);
			if (device)
			{
				if (addGateway)
				{
					deviceList[Util::GenIdDeviceByElement(id, i)] = device;
				}
				if (addDatabase)
				{
					database->DeviceAdd(device);
				}
			}
		}
		device = new DeviceBleSwitchTouchRgb(id, name, mac, data, addr, type, version, 2);
		break;
	case BLE_SWITCH_RGB_3:
	case BLE_SWITCH_RGB_3_SQUARE:
		for (int i = 1; i < 3; i++)
		{
			device = new DeviceBleSwitchTouchRgb(Util::GenIdDeviceByElement(id, i), name, mac, data, addr + i, type, version);
			if (device)
			{
				if (addGateway)
				{
					deviceList[Util::GenIdDeviceByElement(id, i)] = device;
				}
				if (addDatabase)
				{
					database->DeviceAdd(device);
				}
			}
		}
		device = new DeviceBleSwitchTouchRgb(id, name, mac, data, addr, type, version, 3);
		break;
	case BLE_SWITCH_RGB_4:
	case BLE_SWITCH_RGB_4_SQUARE:
		for (int i = 1; i < 4; i++)
		{
			device = new DeviceBleSwitchTouchRgb(Util::GenIdDeviceByElement(id, i), name, mac, data, addr + i, type, version);
			if (device)
			{
				if (addGateway)
				{
					deviceList[Util::GenIdDeviceByElement(id, i)] = device;
				}
				if (addDatabase)
				{
					database->DeviceAdd(device);
				}
			}
		}
		device = new DeviceBleSwitchTouchRgb(id, name, mac, data, addr, type, version, 4);
		break;
	case BLE_SWITCH_ELECTRICAL_1:
	case BLE_SWITCH_ELECTRICAL_WATER_HEATER:
		device = new DeviceBleSwitchElectrical(id, name, mac, data, addr, type, version, 1);
		break;
	case BLE_SWITCH_ELECTRICAL_2:
		for (int i = 1; i < 2; i++)
		{
			device = new DeviceBleSwitchElectrical(Util::GenIdDeviceByElement(id, i), name, mac, data, addr + i, type, version);
			if (device)
			{
				if (addGateway)
				{
					deviceList[Util::GenIdDeviceByElement(id, i)] = device;
				}
				if (addDatabase)
				{
					database->DeviceAdd(device);
				}
			}
		}
		device = new DeviceBleSwitchElectrical(id, name, mac, data, addr, type, version, 2);
		break;
	case BLE_SWITCH_ELECTRICAL_3:
		for (int i = 1; i < 3; i++)
		{
			device = new DeviceBleSwitchElectrical(Util::GenIdDeviceByElement(id, i), name, mac, data, addr + i, type, version);
			if (device)
			{
				if (addGateway)
				{
					deviceList[Util::GenIdDeviceByElement(id, i)] = device;
				}
				if (addDatabase)
				{
					database->DeviceAdd(device);
				}
			}
		}
		device = new DeviceBleSwitchElectrical(id, name, mac, data, addr, type, version, 3);
		break;
	case BLE_SWITCH_ELECTRICAL_4:
		for (int i = 1; i < 4; i++)
		{
			device = new DeviceBleSwitchElectrical(Util::GenIdDeviceByElement(id, i), name, mac, data, addr + i, type, version);
			if (device)
			{
				if (addGateway)
				{
					deviceList[Util::GenIdDeviceByElement(id, i)] = device;
				}
				if (addDatabase)
				{
					database->DeviceAdd(device);
				}
			}
		}
		device = new DeviceBleSwitchElectrical(id, name, mac, data, addr, type, version, 4);
		break;
	case BLE_DC_SCENE_CONTACT:
	case BLE_REMOTE_M3:
	case BLE_REMOTE_M3_V2:
	case BLE_REMOTE_M4:
		device = new DeviceBleSwitchScene6DC(id, name, mac, data, addr, type, version);
		break;
	case BLE_AC_SCENE_CONTACT:
		device = new DeviceBleSwitchScene6AC(id, name, mac, data, addr, version);
		break;
	case BLE_AC_SCENE_CONTACT_RGB:
	case BLE_AC_SCENE_CONTACT_RGB_SQUARE:
		device = new DeviceBleSwitchScene6ACRgb(id, name, mac, data, addr, type, version);
		break;
	case BLE_TEMP_HUM_SENSOR:
		device = new DeviceBleSensorTempHum(id, name, mac, data, addr, version);
		break;
	case BLE_PM_SENSOR:
		device = new DeviceBleSensorPm(id, name, mac, data, addr, version);
		break;
	case BLE_PIR_LIGHT_SENSOR_DC:
		device = new DeviceBlePirLightSensorDC(id, name, mac, data, addr, version);
		break;
	case BLE_PIR_LIGHT_SENSOR_AC:
	case BLE_PIR_LIGHT_SENSOR_AC_AMTRAN:
		device = new DeviceBlePirLightSensorAC(id, name, mac, data, addr, version);
		break;
	case BLE_SMOKE_SENSOR:
		device = new DeviceBleSmokeSensor(id, name, mac, data, addr, version);
		break;
	case BLE_DOOR_SENSOR:
		device = new DeviceBleDoorSensor(id, name, mac, data, addr, version);
		break;
	case BLE_AC_SCENE_SCREEN_TOUCH:
		device = new DeviceBleScreenTouch(id, name, mac, data, addr, version);
		numScreenTouchs++;
		break;
	case BLE_SWITCH_CURTAIN:
	case BLE_SWITCH_RGB_CURTAIN:
	case BLE_SWITCH_RGB_CURTAIN_SQUARE:
		device = new DeviceBleCurtain(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_ROOLING_DOOR:
		device = new DeviceBleRoolDoor(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_1:
	case BLE_SWITCH_WATER_HEATER:
		device = new DeviceBleSwitchTouch(id, name, mac, data, addr, type, version, 1);
		break;
	case BLE_SWITCH_2:
		device = new DeviceBleSwitchTouch(id, name, mac, data, addr, type, version, 2);
		break;
	case BLE_SWITCH_3:
		device = new DeviceBleSwitchTouch(id, name, mac, data, addr, type, version, 3);
		break;
	case BLE_SWITCH_4:
		device = new DeviceBleSwitchTouch(id, name, mac, data, addr, type, version, 4);
		break;

#ifndef ESP_PLATFORM
	case MQTT_AI_HUB:
		device = new DeviceMqttAihub(id, name, mac, data, version);
		break;
#endif

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
		{
			deviceList[id] = device;
		}
		if (addDatabase)
		{
			database->DeviceAdd(device);
		}
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
		{
			groupListMtx.lock();
			groupList[group->GetId()] = group;
			groupListMtx.unlock();
		}
	}
	return group;
}

Rule *Gateway::AddRule(Json::Value &ruleValue, bool addGateway, bool addDatabase)
{
	// TODO: Check Rule id exist
	LOGD("OnAddRule");
	if (ruleValue.isMember("id") && ruleValue["id"].isString() &&
			ruleValue.isMember("name") && ruleValue["name"].isString() &&
			ruleValue.isMember("type") && ruleValue["type"].isString() &&
			ruleValue.isMember("repeat") && ruleValue["repeat"].isInt() &&
			ruleValue.isMember("input") && ruleValue["input"].isObject() &&
			ruleValue.isMember("output") && ruleValue["output"].isArray())
	{
		string id = ruleValue["id"].asString();
		string type = ruleValue["type"].asString();
		int repeat = ruleValue["repeat"].asInt();
		Json::Value inputValue = ruleValue["input"];
		Json::Value outputValues = ruleValue["output"];
		uint32_t addr = 0;
		string name;
		if (ruleValue.isMember("name") && ruleValue["name"].isString())
			name = ruleValue["name"].asString();
		else
			name = id;

		Rule *rule = NULL;
		if (ruleValue.isMember("time") && ruleValue["time"].isObject())
		{
			Json::Value timeValue = ruleValue["time"];
			if (timeValue.isMember("start") && timeValue["start"].isString() &&
					timeValue.isMember("end") && timeValue["end"].isString())
			{
				string startTime = timeValue["start"].asString();
				string endTime = timeValue["end"].asString();
				rule = new Rule(id, type, repeat, name, 0, Util::ConvertStrTimeToInt(startTime), Util::ConvertStrTimeToInt(endTime), ruleValue);
				ruleListMtx.lock();
				ruleList[rule->GetId()] = rule;
				ruleListMtx.unlock();
			}
		}
		if (!rule)
		{
			rule = new Rule(id, type, repeat, name, 0, ruleValue);
			ruleListMtx.lock();
			ruleList[rule->GetId()] = rule;
			ruleListMtx.unlock();
		}

		if (rule)
		{
			rule->SetStatus(true);
			rule->UpdateData(ruleValue);
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
						RuleInputTimer *ruleInputTimer = new RuleInputTimer(rule, timer, repeat);
						rule->AddRuleInput(ruleInputTimer);
					}
				}
			}
			if (inputValue.isMember("device") && inputValue["device"].isArray())
			{
				Json::Value deviceRuleInputList = inputValue["device"];
				for (Json::Value::ArrayIndex i = 0; i < deviceRuleInputList.size(); i++)
				{
					Json::Value deviceRuleInputValue = deviceRuleInputList[i];
					if (deviceRuleInputValue.isObject())
					{
						if (deviceRuleInputValue.isMember("mac") && deviceRuleInputValue["mac"].isString() &&
								deviceRuleInputValue.isMember("data") && deviceRuleInputValue["data"].isObject())
						{
							string id = deviceRuleInputValue["id"].asString();
							Json::Value dataValue = deviceRuleInputValue["data"];
							Device *device = gateway->getDeviceFromMac(mac);
							if (device)
							{
								RuleInputDevice *ruleInputDevice = new RuleInputDevice(rule, device, dataValue);
								rule->AddRuleInput(ruleInputDevice);
							}
						}
					}
				}
			}

			for (Json::Value::ArrayIndex i = 0; i < outputValues.size(); i++)
			{
				Json::Value outputValue = outputValues[i];
				if (outputValue.isMember("delay"))
				{
					RuleOutputDelay *ruleOutputDelay = new RuleOutputDelay(outputValue.asInt());
					rule->AddRuleOutput(ruleOutputDelay);
				}
				else if (outputValue.isMember("deviceId"))
				{
					if (outputValue["deviceId"].isString() && outputValue.isMember("data") && outputValue["data"].isObject())
					{
						Json::Value dataValue = outputValue["data"];
						string id = outputValue["deviceId"].asString();
						Device *device = gateway->getDeviceFromId(id);
						if (device)
						{
							RuleOutputDevice *ruleOutputDevice = new RuleOutputDevice(device, dataValue);
							rule->AddRuleOutput(ruleOutputDevice);
						}
					}
				}
				else if (outputValue.isMember("groupId"))
				{
					if (outputValue["groupId"].isString() && outputValue.isMember("data") && outputValue["data"].isObject())
					{
						Json::Value dataValue = outputValue["data"];
						string id = outputValue["groupId"].asString();
						Group *group = gateway->getGroupFromId(id);
						if (group)
						{
							RuleOutputGroup *ruleOutputGroup = new RuleOutputGroup(group, dataValue);
							rule->AddRuleOutput(ruleOutputGroup);
						}
					}
				}
				else if (outputValue.isMember("sceneId"))
				{
					if (outputValue["sceneId"].isString())
					{
						string id = outputValue["sceneId"].asString();
						SceneBle *sceneBle = gateway->getSceneBleFromId(id);
						if (sceneBle)
						{
							RuleOutputSceneBle *ruleOutputSceneBle = new RuleOutputSceneBle(sceneBle);
							rule->AddRuleOutput(ruleOutputSceneBle);
						}
					}
				}
			}
			if (addGateway)
			{
				ruleListMtx.lock();
				ruleList[id] = rule;
				ruleListMtx.unlock();
			}
			if (addDatabase)
			{
				string ruleStr = ruleValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->RuleAdd(rule, ruleStr, 0);
			}
		}
		else
		{
			LOGE("New rule error, out of memory");
		}
		return rule;
	}
	else
	{
		LOGW("Rule format error");
	}
	return NULL;
}

SceneBle *Gateway::AddNewSceneBle(SceneBle *sceneBle, bool addGateway, bool addDatabase)
{
	if (sceneBle)
	{
		if (addDatabase)
		{
			int rs = database->SceneBleAdd(sceneBle);
			if (rs)
			{
				LOGW("rs: %d", rs);
				return NULL;
			}
		}
		if (addGateway)
		{
			sceneBleListMtx.lock();
			sceneBleList[sceneBle->GetId()] = sceneBle;
			sceneBleListMtx.unlock();
		}
	}
	return sceneBle;
}

Room *Gateway::AddNewRoom(Room *room, bool addGateway, bool addDatabase)
{
	if (room)
	{
		if (addGateway)
		{
			roomList[room->GetId()] = room;
		}
		if (addDatabase)
		{
			database->RoomAdd(room);
		}
	}
	AddNewGroup(room, addGateway, false);
	return room;
}

uint16_t Gateway::getBleAddr()
{
	return ble_addr;
}

uint32_t Gateway::getBleIvIndex()
{
	return ble_iv_index;
}

string Gateway::getBleNetKey()
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

string Gateway::getMac()
{
	return mac;
}

void Gateway::setBleAddr(uint16_t addr)
{
	this->ble_addr = addr;
}

void Gateway::setBleIvIndex(uint32_t ivIndex)
{
	this->ble_iv_index = ivIndex;
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

void Gateway::setMac(string mac)
{
	this->mac = mac;
}

void Gateway::setVersion(string version)
{
	this->version = version;
}

void Gateway::setName(string name)
{
}

int Gateway::OnRpcSetPwMqttOnline(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "SET_PASSWD_MQTT_ONLINE";
		Json::Value dataJsonRsp = Json::objectValue;
		int status = 0;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("PASSWD") && dataValue["PASSWD"].isString())
		{
			string password = dataValue["PASSWD"].asString();
			string user = "";

#ifdef ESP_PLATFORM
			user = "minihub-" + mac;
#else
			user = "hc-" + mac;
#endif

			if (config->SetClientId(user))
			{
				if (config->SetUsername(user))
				{
					if (config->SetPort(8883))
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
			}
			else
			{
				status = 0;
			}
			dataJsonRsp["STATUS"] = status;
			respValue["DATA"] = dataJsonRsp;
			return CODE_EXIT;
		}
	}
	return CODE_ERROR;
}

void Gateway::DelAllDevice()
{
	deviceListMtx.lock();
	for (auto &[id, device] : deviceList)
		delete device;
	deviceList.clear();
	deviceListMtx.unlock();
}

void Gateway::DelAllGroup()
{
	groupListMtx.lock();
	for (auto &[id, group] : groupList)
		delete group;
	groupList.clear();
	groupListMtx.unlock();
}
void Gateway::DelAllSceneBle()
{
	sceneBleListMtx.lock();
	for (auto &[id, scene] : sceneBleList)
		delete scene;
	sceneBleList.clear();
	sceneBleListMtx.unlock();
}

void Gateway::DelAllRule()
{
	ruleListMtx.lock();
	for (auto &[id, rule] : ruleList)
		delete rule;
	ruleList.clear();
	ruleListMtx.unlock();
}
void Gateway::DelAllRoom()
{
	roomListMtx.lock();
	for (auto &[id, room] : roomList)
		delete room;
	roomList.clear();
	roomListMtx.unlock();
}

int Gateway::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
#ifdef __ANDROID__
	int onoff = 0;
	if (dataValue.isObject())
	{
		if (dataValue.isMember(KEY_ATTRIBUTE_RELAY "0") && dataValue[KEY_ATTRIBUTE_RELAY "0"].isInt())
		{
			onoff = dataValue[KEY_ATTRIBUTE_RELAY "0"].asInt();
			if (onoff)
			{
				Util::ExecuteCMD("/system/bin/echo 1 > /sys/class/gpio/gpio114/value");
			}
			else
			{
				Util::ExecuteCMD("/system/bin/echo 0 > /sys/class/gpio/gpio114/value");
			}
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_RELAY "1") && dataValue[KEY_ATTRIBUTE_RELAY "1"].isInt())
		{
			onoff = dataValue[KEY_ATTRIBUTE_RELAY "1"].asInt();
			if (onoff)
			{
				Util::ExecuteCMD("/system/bin/echo 1 > /sys/class/gpio/gpio115/value");
			}
			else
			{
				Util::ExecuteCMD("/system/bin/echo 0 > /sys/class/gpio/gpio115/value");
			}
		}
		return CODE_OK;
	}
#endif
	return CODE_FORMAT_ERROR;
}

int Gateway::pushDeviceUpdateLocal(Json::Value &dataValue)
{
	return PublishToLocalMessage("deviceUpdate", dataValue, "deviceUpdateRsp", NULL, 0);
}

int Gateway::pushDeviceUpdateCloud(Json::Value &dataValue)
{
	return PublishToCloudMessage("deviceUpdate", dataValue, "deviceUpdateRsp", NULL);
}

int Gateway::pushNewDeviceCloud(Json::Value &dataValue)
{
	return PublishToCloudMessage("newDev", dataValue, "newDevRsp", NULL);
}

int Gateway::pushNewDeviceLocal(Json::Value &dataValue)
{
	return PublishToLocalMessage("newDev", dataValue, "newDevRsp", NULL, 0);
}
