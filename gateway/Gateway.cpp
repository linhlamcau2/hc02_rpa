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
#include "DeviceBlePirLightSensorAC_CB09.h"
#include "DeviceBleSmokeSensor.h"
#include "DeviceBleDoorSensor.h"
#include "DeviceBleScreenTouch.h"
#include "DeviceBleCurtain.h"
#include "DeviceBleRoolDoor.h"
#include "DeviceBleSwitchTouch.h"
#include "DeviceBleSwitchCeiling.h"
#include "DeviceBleRepeater.h"
#include "DeviceBleRadaSensorAc.h"
#include "DeviceBleSeftPowerRemote.h"

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
	this->data = "";
	this->isAutoOta = true;
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

SceneDelay *Gateway::getSceneDelayFromId(string id)
{
	sceneDelayListMtx.lock();
	if (sceneDelayList.find(id) != sceneDelayList.end())
	{
		sceneDelayListMtx.unlock();
		return sceneDelayList[id];
	}
	sceneDelayListMtx.unlock();
	return NULL;
}
void Gateway::delSceneDelay(SceneDelay *sceneDelay)
{
	sceneDelayListMtx.lock();
	sceneDelayList.erase(sceneDelay->GetId());
	sceneDelayListMtx.unlock();
	database->SceneDelayDel(sceneDelay);
	delete sceneDelay;
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
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (xTaskCreate(startUdpThread, "Udp", 5120, this, 7, NULL) != pdPASS)
	{
		LOGE("Failed to create task");
		SetLedService(false);
	}
	vTaskDelay(10);
#endif
	// if (xTaskCreate(startCheckOnlineThread, "CheckOnline", 5120, this, 7, NULL) != pdPASS)
	// {
	// 	LOGE("Failed to create task");
	// 	SetLedService(false);
	// }
	// vTaskDelay(10);
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
	database->SceneDelayRead();
	if (gateway->getId().compare("") == 0)
	{
		id = mac;
		gateway->setId(id);
		database->GatewayAdd(gateway);
		database->GatewayRead();
	}

	// Check old version to do something
	string firmwareVersionCurrent = STR(VERSION);
	LOGI("Version: %s", firmwareVersionCurrent.c_str());
	if (getVersion() != firmwareVersionCurrent)
	{
		int addColumnSuccess = database->checkAndAddColumn("Gateway", "data", "TEXT");
		if (addColumnSuccess == CODE_OK || addColumnSuccess == CODE_EXIST)
		{
			database->GatewayUpdateVersion(this, firmwareVersionCurrent);
			sleep(5);
			exit(1);
		}
	}

	Json::Value dataGateway;
	dataGateway.parse(getData());
	if (dataGateway.isObject() && dataGateway.isMember("isAutoOta") && dataGateway["isAutoOta"].isBool())
	{
		this->setAutoOta(dataGateway["isAutoOta"].asBool());
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
	string rmDb = "rm " + DB_NAME;
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
/*
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
*/

/*
void Gateway::PushTelemetryAllLights()
{
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		if (((device->GetType() / 10000) == 1))
		{
			bleProtocol->GetOnoffLight(device->GetAddr());
		}
	}
	deviceListMtx.unlock();
}
*/

void Gateway::CheckAutoOta()
{
	if (this->getAutoOta() & (Util::GetCurrentTimer() == 1))
	{
		Json::Value data;
		data["cmd"] = "checkAutoOta";
		data["rqi"] = Util::genRandRQI(16);
		data["data"]["mac"] = this->getMac();
		data["data"]["version"] = this->getVersion();
#ifdef ESP_PLATFORM
		data["data"]["type"] = 2;
#elif defined(__OPENWRT__)
		data["data"]["type"] = 1;
#else
		data["data"]["type"] = 3;
#endif
		CloudPublish(data);
	}
}

static string ST_array_icon[18] = {"01d", "02d", "03d", "04d", "09d", "10d", "11d", "13d", "50d", "01n", "02n", "03n", "04n", "09n", "10n", "11n", "13n", "50n"};

int Gateway::CheckOnlineThread()
{
	LOGI("Start CheckOnlineThread");
	time_t currentTime = 0;
	time_t oldTime = 0;
	uint32_t allTimeCheck = 0; // time total in a loop check
	bool deviceStateChange = false;

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	Json::Value devicesData;
	Json::Value onlineValue;
	Json::Value offlineValue;
	offlineValue["stt"] = 0;
	onlineValue["stt"] = 1;
#else
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
#endif

	while (!bleProtocol)
	{
		sleep(1);
	}

	while (1)
	{
		if (!bleProtocol->IsProvision() && !LocalProtocol::IsBusy() && !CloudProtocol::IsBusy())
		{
			if (getCheckStatusLights())
			{
				setCheckStatusLights(false);
				for (const auto &[id, device] : deviceList)
				{
					if (((device->GetType() / 10000) == 1) /*|| ((device->GetType() / 1000) == 22) || ((device->GetType() / 1000) == 24) || ((device->GetType() / 1000) == 26)*/)
					{
						bleProtocol->GetOnoffLight(device->GetAddr());
					}
				}
			}
		}
		// Check have device screen touch -> send datetime, weather data
		if ((time(NULL) - oldTime) > 18000)
		{
			oldTime = time(NULL);
			HTTPRequest *httpRequest = new HTTPRequest();
			string dataWeather = httpRequest->GetWeather(Util::GetLatitude(gateway->getData()), Util::GetLongitude(gateway->getData()));
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
						Util::SetStatusWeatherOutdoor(status);
						Util::SetTempWeatherOutdoor(temp);
					}
				}
			}
		}

		if (!bleProtocol->IsProvision() && !LocalProtocol::IsBusy() && !CloudProtocol::IsBusy())
		{
			// deviceListMtx.lock();
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
			devicesData = Json::Value::null;
#endif
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
									// LOGI("Device 0x%04X offline", device->GetAddr());
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
									// LOGI("Device 0x%04X offline", device->GetAddr());
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
									// LOGI("Device 0x%04X online", device->GetAddr());
									device->lastOnlineState = true;
									deviceStateChange = true;
								}
							}
							else
							{
								// trong ngay co ban tin thi online
								if ((device->lastTimeActive + 60 * 60 * 24) >= currentTime)
								{
									// LOGI("Device 0x%04X online", device->GetAddr());
									device->lastOnlineState = true;
									deviceStateChange = true;
								}
							}
						}
						// send device state to server
						if (deviceStateChange)
						{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
							Json::Value deviceData;
							deviceData["id"] = device->GetId();
							if (device->lastOnlineState)
								deviceData["data"] = onlineValue;
							else
								deviceData["data"] = offlineValue;
							devicesData.append(deviceData);
#else
							onlineValue["DATA"][0]["DEVICE_ID"] = device->GetId();
							onlineValue["DATA"][0]["PROPERTIES"][0]["VALUE"] = (int)device->lastOnlineState;
							LocalPublish(onlineValue);
							CloudPublish(onlineValue);
#endif
						}
					}
				}
				else
					break;
			}
			// deviceListMtx.unlock();
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
			if (!devicesData.isNull())
			{
				Json::Value dataValue;
				dataValue["device"] = devicesData;
				gateway->pushDeviceUpdateLocalV2(dataValue);
				gateway->pushDeviceUpdateCloudV2(dataValue);
			}
#endif
		}

		CheckAutoOta(); // Check ota with cloud
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

void Gateway::setCheckStatusLights(bool status)
{
	this->isCheckStatusLights = status;
}

bool Gateway::getCheckStatusLights()
{
	return this->isCheckStatusLights;
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

static Json::Value PushNewDevMqttV2(Device *newDev)
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
	dataValue["DEVICE_UNICAST_ID"] = (uint16_t)scanDevice->GetAddr();
	dataValue["DEVICE_TYPE_ID"] = (uint32_t)scanDevice->GetType();
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
	if (scanDevice->GetType() == BLE_SWITCH_RGB_2 ||
		scanDevice->GetType() == BLE_SWITCH_RGB_2_SQUARE ||
		scanDevice->GetType() == BLE_SWITCH_ELECTRICAL_2 ||
		scanDevice->GetType() == BLE_SWITCH_RGB_2_V2 ||
		scanDevice->GetType() == BLE_SWITCH_RGB_2_SQUARE_V2 ||
		scanDevice->GetType() == BLE_SWITCH_2_CEILING)
	{
		dataValue["PARENT_DEVICE_ID"] = scanDevice->GetId();
		dataValue["DEVICE_ID"] = Util::GenIdDeviceByElement(scanDevice->GetId(), 1, Util::checkGenIdDeviceChild(scanDevice->GetData(), KEYJSON_GEN_DEVICEID));
		dataValue["DEVICE_UNICAST_ID"] = (uint16_t)scanDevice->GetAddr() + 1;
		dataValue["BUTTON_ID"] = 12;
		jsonValue["DATA"] = dataValue;
		LocalPublish(jsonValue);
	}
	else if (scanDevice->GetType() == BLE_SWITCH_RGB_3 ||
			 scanDevice->GetType() == BLE_SWITCH_RGB_3_SQUARE ||
			 scanDevice->GetType() == BLE_SWITCH_ELECTRICAL_3 ||
			 scanDevice->GetType() == BLE_SWITCH_RGB_3_V2 ||
			 scanDevice->GetType() == BLE_SWITCH_RGB_3_SQUARE_V2 ||
			 scanDevice->GetType() == BLE_SWITCH_3_CEILING)
	{
		for (int i = 1; i <= 2; i++)
		{
			dataValue["PARENT_DEVICE_ID"] = scanDevice->GetId();
			dataValue["DEVICE_ID"] = Util::GenIdDeviceByElement(scanDevice->GetId(), i, Util::checkGenIdDeviceChild(scanDevice->GetData(), KEYJSON_GEN_DEVICEID));
			dataValue["DEVICE_UNICAST_ID"] = (uint16_t)scanDevice->GetAddr() + i;
			dataValue["BUTTON_ID"] = 11 + i;
			jsonValue["DATA"] = dataValue;
			LocalPublish(jsonValue);
		}
	}
	else if (scanDevice->GetType() == BLE_SWITCH_RGB_4 ||
			 scanDevice->GetType() == BLE_SWITCH_RGB_4_SQUARE ||
			 scanDevice->GetType() == BLE_SWITCH_ELECTRICAL_4 ||
			 scanDevice->GetType() == BLE_SWITCH_RGB_4_V2 ||
			 scanDevice->GetType() == BLE_SWITCH_RGB_4_SQUARE_V2)
	{
		for (int i = 1; i <= 3; i++)
		{
			dataValue["PARENT_DEVICE_ID"] = scanDevice->GetId();
			dataValue["DEVICE_ID"] = Util::GenIdDeviceByElement(scanDevice->GetId(), i, Util::checkGenIdDeviceChild(scanDevice->GetData(), KEYJSON_GEN_DEVICEID));
			dataValue["DEVICE_UNICAST_ID"] = (uint16_t)scanDevice->GetAddr() + i;
			dataValue["BUTTON_ID"] = 11 + i;
			jsonValue["DATA"] = dataValue;
			LocalPublish(jsonValue);
		}
	}
	else if (scanDevice->GetType() == BLE_SWITCH_5_CEILING)
	{
		for (int i = 1; i <= 4; i++)
		{
			dataValue["PARENT_DEVICE_ID"] = scanDevice->GetId();
			dataValue["DEVICE_ID"] = Util::GenIdDeviceByElement(scanDevice->GetId(), i, Util::checkGenIdDeviceChild(scanDevice->GetData(), KEYJSON_GEN_DEVICEID));
			dataValue["DEVICE_UNICAST_ID"] = (uint16_t)scanDevice->GetAddr() + i;
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
			dataValue["DEVICE_ID"] = Util::GenIdDeviceByElement(scanDevice->GetId(), i, Util::checkGenIdDeviceChild(scanDevice->GetData(), KEYJSON_GEN_DEVICEID));
			dataValue["DEVICE_UNICAST_ID"] = (uint16_t)scanDevice->GetAddr();
			dataValue["BUTTON_ID"] = 11 + i;
			jsonValue["DATA"] = dataValue;
			LocalPublish(jsonValue);
		}
	}
	else if (scanDevice->GetType() == BLE_SEFTPOWER_REMOTE_1 ||
			 scanDevice->GetType() == BLE_SEFTPOWER_REMOTE_2 ||
			 scanDevice->GetType() == BLE_SEFTPOWER_REMOTE_3)
	{
		DeviceBleSeftPowerRemote *deviceBleSeftPowerRemote = dynamic_cast<DeviceBleSeftPowerRemote *>(scanDevice);
		if (deviceBleSeftPowerRemote->GetParent())
		{
			dataValue["PARENT_DEVICE_ID"] = deviceBleSeftPowerRemote->GetParent()->GetId();
			dataValue["DEVICE_ID"] = deviceBleSeftPowerRemote->GetId();
			dataValue["DEVICE_UNICAST_ID"] = (uint16_t)deviceBleSeftPowerRemote->GetAddr();
			dataValue["BUTTON_ID"] = 11;
			jsonValue["DATA"] = dataValue;
			LocalPublish(jsonValue);
		}
	}
#endif
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	Json::Value jsonData = PushNewDevMqttV2(scanDevice);
	pushNewDeviceLocalV2(jsonData);
#endif
}

Device *Gateway::AddNewDevice(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version, bool addGateway, bool addDatabase)
{
	LOGI("Add new device id: %s, name: %s, mac: %s, addr: 0x%04X, type: 0x%04X, verion: %d", id.c_str(), name.c_str(), mac.c_str(), addr, type, version);
	Device *device = NULL;
	string deviceChildId = "";
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
	case BLE_LED_OP_TRAN_40W:
	case BLE_LED_OP_TUONG:
	case BLE_LED_OP_TRAN_LOA:
	case BLE_PANEL_TRON:
	case BLE_PANEL_VUONG:
	case BLE_TRACKLIGHT:
	case BLE_LED_THA_TRAN:
	case BLE_LED_TUBE_M16:
	case BLE_LED_RLT03_06W:
	case BLE_LED_RLT02_10W:
	case BLE_LED_RLT02_20W:
	case BLE_LED_RLT01_10W:
	case BLE_LED_TRL08_20W:
	case BLE_LED_TRL08_10W:
	case BLE_LED_RLT03_12W:
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
	case BLE_SWITCH_RGB_SOCKET_1:
	case BLE_SWITCH_RGB_1_V2:
	case BLE_SWITCH_RGB_1_SQUARE_V2:
		device = new DeviceBleSwitchTouchRgb(id, name, mac, data, addr, type, version, 1);
		break;
	case BLE_SWITCH_RGB_2:
	case BLE_SWITCH_RGB_2_SQUARE:
	case BLE_SWITCH_RGB_2_V2:
	case BLE_SWITCH_RGB_2_SQUARE_V2:
		for (int i = 1; i < 2; i++)
		{
			deviceChildId = Util::GenIdDeviceByElement(id, i, Util::checkGenIdDeviceChild(data, KEYJSON_GEN_DEVICEID));
			device = new DeviceBleSwitchTouchRgb(deviceChildId, name, mac, data, addr + i, type, version);
			if (device)
			{
				device->lastTimeActive = time(NULL);
				if (addGateway)
				{
					deviceList[deviceChildId] = device;
				}
			}
		}
		device = new DeviceBleSwitchTouchRgb(id, name, mac, data, addr, type, version, 2);
		break;
	case BLE_SWITCH_RGB_3:
	case BLE_SWITCH_RGB_3_SQUARE:
	case BLE_SWITCH_RGB_3_V2:
	case BLE_SWITCH_RGB_3_SQUARE_V2:
		for (int i = 1; i < 3; i++)
		{
			deviceChildId = Util::GenIdDeviceByElement(id, i, Util::checkGenIdDeviceChild(data, KEYJSON_GEN_DEVICEID));
			device = new DeviceBleSwitchTouchRgb(deviceChildId, name, mac, data, addr + i, type, version);
			if (device)
			{
				device->lastTimeActive = time(NULL);
				if (addGateway)
				{
					deviceList[deviceChildId] = device;
				}
			}
		}
		device = new DeviceBleSwitchTouchRgb(id, name, mac, data, addr, type, version, 3);
		break;
	case BLE_SWITCH_RGB_4:
	case BLE_SWITCH_RGB_4_SQUARE:
	case BLE_SWITCH_RGB_4_V2:
	case BLE_SWITCH_RGB_4_SQUARE_V2:
		for (int i = 1; i < 4; i++)
		{
			deviceChildId = Util::GenIdDeviceByElement(id, i, Util::checkGenIdDeviceChild(data, KEYJSON_GEN_DEVICEID));
			device = new DeviceBleSwitchTouchRgb(deviceChildId, name, mac, data, addr + i, type, version);
			if (device)
			{
				device->lastTimeActive = time(NULL);
				if (addGateway)
				{
					deviceList[deviceChildId] = device;
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
			deviceChildId = Util::GenIdDeviceByElement(id, i, Util::checkGenIdDeviceChild(data, KEYJSON_GEN_DEVICEID));
			device = new DeviceBleSwitchElectrical(deviceChildId, name, mac, data, addr + i, type, version);
			if (device)
			{
				device->lastTimeActive = time(NULL);
				if (addGateway)
				{
					deviceList[deviceChildId] = device;
				}
			}
		}
		device = new DeviceBleSwitchElectrical(id, name, mac, data, addr, type, version, 2);
		break;
	case BLE_SWITCH_ELECTRICAL_3:
		for (int i = 1; i < 3; i++)
		{
			deviceChildId = Util::GenIdDeviceByElement(id, i, Util::checkGenIdDeviceChild(data, KEYJSON_GEN_DEVICEID));
			device = new DeviceBleSwitchElectrical(deviceChildId, name, mac, data, addr + i, type, version);
			if (device)
			{
				device->lastTimeActive = time(NULL);
				if (addGateway)
				{
					deviceList[deviceChildId] = device;
				}
			}
		}
		device = new DeviceBleSwitchElectrical(id, name, mac, data, addr, type, version, 3);
		break;
	case BLE_SWITCH_ELECTRICAL_4:
		for (int i = 1; i < 4; i++)
		{
			deviceChildId = Util::GenIdDeviceByElement(id, i, Util::checkGenIdDeviceChild(data, KEYJSON_GEN_DEVICEID));
			device = new DeviceBleSwitchElectrical(deviceChildId, name, mac, data, addr + i, type, version);
			if (device)
			{
				device->lastTimeActive = time(NULL);
				if (addGateway)
				{
					deviceList[deviceChildId] = device;
				}
			}
		}
		device = new DeviceBleSwitchElectrical(id, name, mac, data, addr, type, version, 4);
		break;
	case BLE_SWITCH_2_CEILING:
		for (int i = 1; i < 2; i++)
		{
			deviceChildId = Util::GenIdDeviceByElement(id, i, Util::checkGenIdDeviceChild(data, KEYJSON_GEN_DEVICEID));
			device = new DeviceBleSwitchCeiling(deviceChildId, name, mac, data, addr + i, type, version);
			if (device)
			{
				device->lastTimeActive = time(NULL);
				if (addGateway)
				{
					deviceList[deviceChildId] = device;
				}
			}
		}
		device = new DeviceBleSwitchCeiling(id, name, mac, data, addr, type, version, 2);
		break;
	case BLE_SWITCH_3_CEILING:
		for (int i = 1; i < 3; i++)
		{
			deviceChildId = Util::GenIdDeviceByElement(id, i, Util::checkGenIdDeviceChild(data, KEYJSON_GEN_DEVICEID));
			device = new DeviceBleSwitchCeiling(deviceChildId, name, mac, data, addr + i, type, version);
			if (device)
			{
				device->lastTimeActive = time(NULL);
				if (addGateway)
				{
					deviceList[deviceChildId] = device;
				}
			}
		}
		device = new DeviceBleSwitchCeiling(id, name, mac, data, addr, type, version, 3);
		break;
	case BLE_SWITCH_5_CEILING:
		for (int i = 1; i < 5; i++)
		{
			deviceChildId = Util::GenIdDeviceByElement(id, i, Util::checkGenIdDeviceChild(data, KEYJSON_GEN_DEVICEID));
			device = new DeviceBleSwitchCeiling(deviceChildId, name, mac, data, addr + i, type, version);
			if (device)
			{
				device->lastTimeActive = time(NULL);
				if (addGateway)
				{
					deviceList[deviceChildId] = device;
				}
			}
		}
		device = new DeviceBleSwitchCeiling(id, name, mac, data, addr, type, version, 5);
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
	case BLE_PIR_LIGHT_SENSOR_DC_CB09:
	case BLE_PIR_LIGHT_SENSOR_DC_CB10:
		device = new DeviceBlePirLightSensorDC(id, name, mac, data, addr, type, version);
		break;
	case BLE_RADA_LIGHT_SENSOR_AC_CB15:
		device = new DeviceBleRadaSensorAc(id, name, mac, data, addr, type, version);
		break;
	case BLE_PIR_LIGHT_SENSOR_AC:
		device = new DeviceBlePirLightSensorAC(id, name, mac, data, addr, type, version);
		break;
	case BLE_PIR_LIGHT_SENSOR_AC_AMTRAN:
		device = new DeviceBlePirLightSensorAC_CB09(id, name, mac, data, addr, type, version);
		break;
	case BLE_SMOKE_SENSOR:
		device = new DeviceBleSmokeSensor(id, name, mac, data, addr, version);
		break;
	case BLE_DOOR_SENSOR:
	case BLE_DOOR_CB16_SENSOR:
		device = new DeviceBleDoorSensor(id, name, mac, data, addr, version);
		break;
	case BLE_AC_SCENE_SCREEN_TOUCH:
		device = new DeviceBleScreenTouch(id, name, mac, data, addr, version);
		break;
	case BLE_SWITCH_CURTAIN:
	case BLE_SWITCH_RGB_CURTAIN:
	case BLE_SWITCH_RGB_CURTAIN_SQUARE:
	case BLE_SWITCH_RGB_CURTAIN_HCN:
	case BLE_SWITCH_RGB_CURTAIN_SQUARE_V2:
		device = new DeviceBleCurtain(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_ROOLING_DOOR:
	case BLE_SWITCH_ROOLING_DOOR_V2:
	case BLE_SWITCH_ROOLING_DOOR_SQUARE:
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
	case BLE_REPEATER:
		device = new DeviceBleRepeater(id, name, mac, data, addr, type, version);
		break;
	case BLE_SEFTPOWER_REMOTE_1:
	case BLE_SEFTPOWER_REMOTE_2:
	case BLE_SEFTPOWER_REMOTE_3:
		device = getDeviceFromId(id);
		if (device)
		{
			delDevice(device);
		}
		device = new DeviceBleSeftPowerRemote(id, name, mac, data, addr, type, version, NULL);
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
		device->lastTimeActive = time(NULL);
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

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
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
		Json::Value outputValue = ruleValue["output"];
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
			int delayTime = 0;
			for (Json::Value::ArrayIndex i = 0; i < outputValue.size(); i++)
			{
				Json::Value temp_outputValue = outputValue[i];
				if (temp_outputValue.isMember("delay"))
				{
					delayTime = temp_outputValue.asInt();
				}
				else if (temp_outputValue.isMember("deviceId"))
				{
					if (temp_outputValue["deviceId"].isString() && temp_outputValue.isMember("data") && temp_outputValue["data"].isObject())
					{
						Json::Value dataValue = temp_outputValue["data"];
						string id = temp_outputValue["deviceId"].asString();
						Device *device = gateway->getDeviceFromId(id);
						if (device)
						{
							RuleOutputDevice *ruleOutput = new RuleOutputDevice(device, dataValue, delayTime);
							rule->AddRuleOutput(ruleOutput);
							delayTime = 0;
						}
					}
				}
				else if (temp_outputValue.isMember("groupId"))
				{
					if (temp_outputValue["groupId"].isString() && temp_outputValue.isMember("data") && temp_outputValue["data"].isObject())
					{
						Json::Value dataValue = temp_outputValue["data"];
						string id = temp_outputValue["groupId"].asString();
						Group *group = gateway->getGroupFromId(id);
						if (group)
						{
							RuleOutputGroup *ruleOutput = new RuleOutputGroup(group, dataValue, delayTime);
							rule->AddRuleOutput(ruleOutput);
							delayTime = 0;
						}
					}
				}
				else if (temp_outputValue.isMember("sceneId"))
				{
					if (temp_outputValue["sceneId"].isString() && temp_outputValue.isMember("data") && temp_outputValue["data"].isObject())
					{
						Json::Value dataValue = temp_outputValue["data"];
						string id = temp_outputValue["sceneId"].asString();
						SceneBle *sceneBle = gateway->getSceneBleFromId(id);
						if (sceneBle)
						{
							RuleOutputSceneBle *ruleOutput = new RuleOutputSceneBle(sceneBle, delayTime);
							rule->AddRuleOutput(ruleOutput);
							delayTime = 0;
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
#else
Rule *Gateway::AddRule(Json::Value &ruleValue, bool addGateway, bool addDatabase)
{
	if (ruleValue.isMember("EVENT_TRIGGER_ID") && ruleValue["EVENT_TRIGGER_ID"].isString() &&
		ruleValue.isMember("STATUS") && ruleValue["STATUS"].isInt() &&
		ruleValue.isMember("EACH_DAY") && ruleValue["EACH_DAY"].isArray())
	{
		int status = ruleValue["STATUS"].asInt();
		string id = ruleValue["EVENT_TRIGGER_ID"].asString();
		uint32_t addr = 0;
		string name;
		if (ruleValue.isMember("NAME") && ruleValue["NAME"].isString())
			name = ruleValue["NAME"].asString();
		else
			name = id;

		bool isFirstRun = true;
		if (ruleValue.isMember("isFirstRun") && ruleValue["isFirstRun"].isBool())
			isFirstRun = ruleValue["isFirstRun"].asBool();

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
				else if (repeatDays[i] == "EACHTHURSDAY")
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
			repeat = Util::ConvertRepeatDayToInt(0, 0, 0, 0, 0, 0, 0);
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
		int logical = -1;
		if (ruleValue.isMember("LOGICAL_OPERATOR_ID") && ruleValue["LOGICAL_OPERATOR_ID"].isInt())
			logical = ruleValue["LOGICAL_OPERATOR_ID"].asInt();
		string type = "or";
		if (logical == -1 || logical == 3 || logical == 2) // rule theo thoi gian or theo thoi gian va tb dau vao
		{
			if (logical == 3 || logical == -1)
			{
				type = "and";
			}
			else if (logical == 2)
			{
				type = "or";
			}
			if (ruleValue.isMember("START_AT") && ruleValue["START_AT"].isString())
			{
				string endAt = "";
				if (ruleValue.isMember("END_AT") && ruleValue["END_AT"].isString())
				{
					endAt = ruleValue["END_AT"].asString();
				}
				string startAt = ruleValue["START_AT"].asString();
				rule = new Rule(id, type, repeat, "", 0, Util::ConvertStrTimeToInt(startAt), Util::ConvertStrTimeToInt(endAt), ruleValue, isFirstRun);
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
			if (repeat == Util::ConvertRepeatDayToInt(0, 0, 0, 0, 0, 0, 0))
				repeat = Util::ConvertRepeatDayToInt(1, 1, 1, 1, 1, 1, 1);
			if (logical == 0)
			{
				type = "or";
			}
			else if (logical == 1)
			{
				type = "and";
			}
			rule = new Rule(id, type, repeat, "", 0, ruleValue, isFirstRun);
			if (!rule)
			{
				LOGW("New rule error");
			}
		}
		else if (logical == -2)
		{
			type = "or";
			rule = new Rule(id, type, repeat, "", 0, ruleValue, isFirstRun);
			if (!rule)
			{
				LOGW("New rule error");
			}
		}

		if (rule)
		{
			bool isEnable = (status) ? true : false;
			rule->SetStatus(isEnable);
			rule->UpdateData(ruleValue);
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

						Device *deviceInputRule = getDeviceFromId(devId);
						if (deviceInputRule)
						{
							Json::Value datasDevInput;
							if (devAttribute.isMember("ID") && devAttribute["ID"].isInt() && devAttribute.isMember("VALUE"))
							{
								int id = devAttribute["ID"].asInt();
								string op = "";
								Json::Value values = Json::arrayValue;
								if (devAttribute["VALUE"].isArray())
								{
									values = devAttribute["VALUE"];
									op = "<>";
								}
								else if (devAttribute["VALUE"].isInt())
								{
									values.append(devAttribute["VALUE"].asInt());
									op = "=";
								}

								if (deviceInputRule->GetType() == BLE_SWITCH_RGB_1 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_2 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_3 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_4 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_WATER_HEATER ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_1_SQUARE ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_2_SQUARE ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_3_SQUARE ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_4_SQUARE ||
									deviceInputRule->GetType() == BLE_SWITCH_ELECTRICAL_1 ||
									deviceInputRule->GetType() == BLE_SWITCH_ELECTRICAL_2 ||
									deviceInputRule->GetType() == BLE_SWITCH_ELECTRICAL_3 ||
									deviceInputRule->GetType() == BLE_SWITCH_ELECTRICAL_4 ||
									deviceInputRule->GetType() == BLE_SWITCH_ELECTRICAL_WATER_HEATER ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_SOCKET_1 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_1_V2 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_1_SQUARE_V2 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_2_V2 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_2_SQUARE_V2 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_3_V2 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_3_SQUARE_V2 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_4_V2 ||
									deviceInputRule->GetType() == BLE_SWITCH_RGB_4_SQUARE_V2 ||
									deviceInputRule->GetType() == BLE_SWITCH_2_CEILING ||
									deviceInputRule->GetType() == BLE_SWITCH_3_CEILING ||
									deviceInputRule->GetType() == BLE_SWITCH_5_CEILING)
								{
									if (id == BLE_ATTRIBUTE_BUTTON_1 || id == BLE_ATTRIBUTE_BUTTON_2 || id == BLE_ATTRIBUTE_BUTTON_3 || id == BLE_ATTRIBUTE_BUTTON_4)
									{
										Device *deviceInputRuleChild = getDeviceBleFromAddr(deviceInputRule->GetAddr() + (id - 11));
										if (deviceInputRuleChild)
										{
											// LOGE("Device Id: %s", deviceInputRuleChild->GetId().c_str());
											datasDevInput["ID"] = BLE_ATTRIBUTE_ONOFF;
											datasDevInput["VALUE"] = values;
											datasDevInput["OP"] = op;

											RuleInputDevice *ruleInputDevice = new RuleInputDevice(rule, deviceInputRuleChild, datasDevInput);
											if (ruleInputDevice)
											{
												rule->AddRuleInput(ruleInputDevice);
											}
											else
												LOGW("create rule input device error");
										}
										else
											LOGW("Device child not found");
									}
								}
								else
								{
									datasDevInput["ID"] = id;
									datasDevInput["VALUE"] = values;
									datasDevInput["OP"] = op;

									RuleInputDevice *ruleInputDevice = new RuleInputDevice(rule, deviceInputRule, datasDevInput);
									if (ruleInputDevice)
									{
										rule->AddRuleInput(ruleInputDevice);
									}
									else
										LOGW("create rule input device error");
								}
							}
							else
								LOGW("Device Attribute error");
						}
						else
							LOGW("Device %s does not exist", devId.c_str());
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
							RuleOutputDevice *ruleOutoutDevice = new RuleOutputDevice(deviceOutputRule, property, 0);
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
							LOGW("Device %s does not exist", devIdOp.c_str());
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
							RuleOutputGroup *ruleOutputGroup = new RuleOutputGroup(groupOutputRule, property, 0);
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
							LOGW("Group %s does not exist", groupId.c_str());
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
							int delay = 0;
							if (scenesOutputRule.isMember("TIME") && scenesOutputRule["TIME"].isInt())
								delay = scenesOutputRule["TIME"].asInt();
							string sceneId = scenesOutputRule["SCENE_ID"].asString();
							SceneBle *sceneBleOutputRule = getSceneBleFromId(sceneId);
							if (sceneBleOutputRule)
							{
								RuleOutputSceneBle *ruleOutputSceneBle = new RuleOutputSceneBle(sceneBleOutputRule, delay);
								if (ruleOutputSceneBle)
								{
									rule->AddRuleOutput(ruleOutputSceneBle);
								}
								else
								{
									LOGW("Create rule output scene ble error");
								}
							}
							else
							{
								LOGW("SceneBle %s does not exist", sceneId.c_str());
								SceneDelay *sceneDelayOutputRule = getSceneDelayFromId(sceneId);
								if (sceneDelayOutputRule)
								{
									RuleOutputSceneDelay *ruleOutputSceneDelay = new RuleOutputSceneDelay(sceneDelayOutputRule, delay);
									if (ruleOutputSceneDelay)
									{
										rule->AddRuleOutput(ruleOutputSceneDelay);
									}
									else
									{
										LOGW("Create rule output scene delay error");
									}
								}
							}
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
				ruleValue["isFirstRun"] = true;
				string ruleStr = ruleValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->RuleAdd(rule, ruleStr, status);
			}
		}
		return rule;
	}
	else
	{
		LOGW("Rule error format");
	}
	return NULL;
}
#endif

// Scene *Gateway::HandleRule

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

SceneDelay *Gateway::AddNewSceneDelay(SceneDelay *sceneDelay, bool addGateway, bool addDatabase, bool processData)
{
	if (sceneDelay)
	{
		if (addDatabase)
		{
			int rs = database->SceneDelayAdd(sceneDelay);
			if (rs)
			{
				LOGW("rs: %d", rs);
				return NULL;
			}
		}
		if (addGateway)
		{
			sceneDelayListMtx.lock();
			sceneDelayList[sceneDelay->GetId()] = sceneDelay;
			sceneDelayListMtx.unlock();
		}
		if (processData)
		{
			Json::Value data = sceneDelay->GetData();
			int delay = 0;
			if (data.isMember("DEVICES") && data["DEVICES"].isArray())
			{
				Device *device = NULL;
				for (Json::ArrayIndex i = 0; i < data["DEVICES"].size(); i++)
				{
					Json::Value deviceOutput = data["DEVICES"][i];
					if (deviceOutput.isMember("DELAY") && deviceOutput["DELAY"].isInt())
					{
						delay = deviceOutput["DELAY"].asInt();
					}
					if (deviceOutput.isMember("DEVICE_ID") && deviceOutput["DEVICE_ID"].isString() && deviceOutput.isMember("PROPERTIES") && deviceOutput["PROPERTIES"].isArray())
					{
						string devId = deviceOutput["DEVICE_ID"].asString();
						Json::Value propertyDev = deviceOutput["PROPERTIES"];
						Json::Value propertyDevArr = Util::arrangeJson(propertyDev);
						if (propertyDevArr == Json::Value::null)
							propertyDevArr = propertyDev;
						device = getDeviceFromId(devId);
						if (device)
						{
							SceneDelayDeviceOutput *sceneDelayDeviceOutput = new SceneDelayDeviceOutput(device, propertyDevArr, delay);
							if (sceneDelayDeviceOutput)
							{
								sceneDelay->AddSceneDelayOutput(sceneDelayDeviceOutput);
							}
						}
					}
					else
					{
						LOGW("Data device output error");
					}
				}
			}

			if (data.isMember("GROUPS") && data["GROUPS"].isArray())
			{
				Group *group = NULL;
				for (int j = 0; j < data["GROUPS"].size(); j++)
				{
					Json::Value groupInSceneDelay = data["GROUPS"][j];
					if (groupInSceneDelay.isMember("DELAY") && groupInSceneDelay["DELAY"].isInt())
					{
						delay = groupInSceneDelay["DELAY"].asInt();
					}
					if (groupInSceneDelay.isMember("GROUP_ID") && groupInSceneDelay["GROUP_ID"].isString() && groupInSceneDelay.isMember("PROPERTIES") && groupInSceneDelay["PROPERTIES"].isArray())
					{
						string groupId = groupInSceneDelay["GROUP_ID"].asString();
						Json::Value propertyGr = groupInSceneDelay["PROPERTIES"];
						Json::Value propertyGrArr = Util::arrangeJson(propertyGr);
						if (propertyGrArr == Json::Value::null)
							propertyGrArr = propertyGr;
						group = getGroupFromId(groupId);
						if (group)
						{
							SceneDelayGroupOutput *sceneDelayGroupOutput = new SceneDelayGroupOutput(group, propertyGrArr, delay);
							if (sceneDelayGroupOutput)
							{
								sceneDelay->AddSceneDelayOutput(sceneDelayGroupOutput);
							}
						}
					}
					else
					{
						LOGW("Data Group output error");
					}
				}
			}
		}
	}
	return sceneDelay;
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

string Gateway::getData()
{
	return this->data;
}

string Gateway::getMac()
{
	return mac;
}

bool Gateway::getAutoOta()
{
	return this->isAutoOta;
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

void Gateway::setData(string data)
{
	this->data = data;
}

void Gateway::setAutoOta(bool isAutoOta)
{
	this->isAutoOta = isAutoOta;
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

uint32_t Gateway::getMaxAddrBle()
{
	uint32_t maxAddrBle = 2;
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		if ((device->GetAddr() > maxAddrBle) && (device->GetAddr() < 49152))
		{
			maxAddrBle = device->GetAddr();
			LOGW("max assiged: %d", maxAddrBle);
		}
	}
	deviceListMtx.unlock();
	LOGW("max return: %d", maxAddrBle);
	return maxAddrBle;
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
void Gateway::DelAllSceneDelay()
{
	sceneDelayListMtx.lock();
	for (auto &[id, scene] : sceneDelayList)
		delete scene;
	sceneDelayList.clear();
	sceneDelayListMtx.unlock();
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

void Gateway::printGroup()
{
	for (auto &[id, grp] : groupList)
	{
		LOGI("group: %s", id.c_str());
		for (auto &dev : grp->deviceList)
		{
			LOGI("\tdev:%s: %d", dev->device->GetId().c_str(), dev->device->GetAddr());
		}
	}
}

void Gateway::printScene()
{
	for (auto &[id, sce] : sceneBleList)
	{
		LOGI("scene: %s", id.c_str());
		for (auto &dev : sce->deviceList)
		{
			LOGI("\tdev:%s: %d", dev->device->GetId().c_str(), dev->device->GetAddr());
		}
	}
}
void Gateway::printRoom()
{
	for (auto &[id, rm] : roomList)
	{
		LOGI("room: %s", id.c_str());
		for (auto &dev : rm->deviceList)
		{
			LOGI("\tdev:%s: %d", dev->device->GetId().c_str(), dev->device->GetAddr());
		}
	}
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

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2

int Gateway::pushDeviceUpdateLocalV2(Json::Value &dataValue)
{
	return PublishToLocalMessageV2("deviceUpdate", dataValue, "deviceUpdateRsp", NULL, 0);
}

int Gateway::pushDeviceUpdateCloudV2(Json::Value &dataValue)
{
	return PublishToCloudMessageV2("deviceUpdate", dataValue, "deviceUpdateRsp", NULL);
}

int Gateway::pushNewDeviceCloudV2(Json::Value &dataValue)
{
	return PublishToCloudMessageV2("newDev", dataValue, "newDevRsp", NULL);
}
int Gateway::pushNewDeviceLocalV2(Json::Value &dataValue)
{
	return PublishToLocalMessageV2("newDev", dataValue, "newDevRsp", NULL, 0);
}
#endif
