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
#ifdef ESP_PLATFORM
#include "Config.h"
#include "Led.h"
#endif
#include "Http.h"

#include "RuleInputTimer.h"
#include "RuleOutputGroup.h"
#include "RuleOutputDevice.h"

#include "BleDefine.h"
#include "BleProtocol.h"
#include "DeviceBleAll.h"
#include "DeviceBleSwitchOnoff.h"
#include "DeviceBleLightOnoffCctDim.h"
#include "DeviceBleLightOnoffHslModeRGB.h"
#include "DeviceBleLightOnoffCctDimHslModeRGB.h"
#include "DeviceBleSwitchTouchRgb1.h"
#include "DeviceBleSwitchTouchRgb2.h"
#include "DeviceBleSwitchTouchRgb3.h"
#include "DeviceBleSwitchTouchRgb4.h"
#include "DeviceBleSwitchElectrical1.h"
#include "DeviceBleSwitchElectrical2.h"
#include "DeviceBleSwitchElectrical3.h"
#include "DeviceBleSwitchElectrical4.h"
#include "DeviceBleSwitchScene6DC.h"
#include "DeviceBleSwitchScene6AC.h"
#include "DeviceBleSwitchScene6ACRgb.h"
#include "DeviceBleSensorTempHum.h"
#include "DeviceBleSensorPm.h"
#include "DeviceBlePirLightSensorDC.h"
#include "DeviceBleSmokeSensor.h"
#include "DeviceBleDoorSensor.h"
#include "DeviceBleScreenTouch.h"
#include "DeviceBleCurtain.h"
#include "DeviceBleRoolDoor.h"

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#include "DeviceZigbeeOnoff.h"
#include "DeviceZigbeeTelinkOnoff.h"
#endif

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
	this->ble_addr = 0;
	this->ble_iv_index = 0;
	this->ble_appkey = "";
	this->ble_appkey = "";
	this->ble_devicekey = "";
	this->numScreenTouchs = 0;
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
	if (deviceList.find(id) != deviceList.end())
	{
		deviceListMtx.unlock();
		return deviceList[id];
	}
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
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	initMqttMessageV2();
#endif

#ifdef ESP_PLATFORM
	LOGI("Free memory: %d bytes, internal: %d bytes", esp_get_free_heap_size(), esp_get_free_internal_heap_size());
	if (xTaskCreate(startUdpThread, "Udp", 5120, this, 7, NULL) != pdPASS)
	{
		LOGE("Failed to create task");
		Led::SetLedService(MODE_OFF);
	}
	vTaskDelay(10);
	if (xTaskCreate(startCheckOnlineThread, "CheckOnline", 5120, this, 7, NULL) != pdPASS)
	{
		LOGE("Failed to create task");
		Led::SetLedService(MODE_OFF);
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
	database->SceneDelayRead();
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
		Led::SetModeLedInternet(MODE_ON);
		Led::SetLedInternet(MODE_ON);
#endif
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
	deviceListMtx.lock();
	deviceList.clear();
	deviceListMtx.unlock();

	groupListMtx.lock();
	groupList.clear();
	groupListMtx.unlock();

	ruleListMtx.lock();
	ruleList.clear();
	ruleListMtx.unlock();

	sceneBleListMtx.lock();
	sceneBleList.clear();
	sceneBleListMtx.unlock();

	sceneDelayListMtx.lock();
	sceneDelayList.clear();
	sceneDelayListMtx.unlock();

	roomListMtx.lock();
	roomList.clear();
	roomListMtx.unlock();

	database->DeviceDelAll();
	database->DeviceAttributeDelAll();
	database->DeviceBleChildDelAll();
	database->GroupDelAll();
	database->DeviceInGroupDelAll();
	database->RoomDelAll();
	database->DeviceInRoomDelAll();
	database->SceneBleDelAll();
	database->DeviceInSceneBleDelAll();
	database->RuleDelAll();

	database->GatewayUpdateId(gateway, "");
	database->GatewayUpdateDormitory(gateway, "");
	gateway->setDormitory("");
	gateway->setId("");
	gateway->setBleAppkey("");
	if (bleProtocol)
	{
		bleProtocol->ResetDelAll();
		bleProtocol->ResetFactory();
	}
	else
		LOGW("BleProtocol null");
}

void Gateway::SendDataForScreenTouch(Device *device, string &dataWeather, uint8_t statusWeather, uint16_t temp)
{
	if (bleProtocol)
	{
		bleProtocol->SendDate(device->GetAddr(), Util::GetYearsCurrent(), Util::GetMonthsCurrent(), Util::GetDateCurrent(), Util::GetDaysCurrent());
		bleProtocol->SendTime(device->GetAddr(), Util::GetHoursCurrent(), Util::GetMinutesCurrent(), Util::GetSecondsCurrent());
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
								if ((device->lastTimeCheckActive + allTimeCheck) <= currentTime)
								{
									bleProtocol->SendOnlineCheck(device->GetAddr(), device->GetType(), device->GetVersion());
									device->lastTimeCheckActive = currentTime;
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
				}
			}
			// deviceListMtx.unlock();
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
	dataValue["DEVICE_UNICAST_ID"] = (int)scanDevice->GetAddr();
	dataValue["DEVICE_TYPE_ID"] = (int)scanDevice->GetType();
	dataValue["MAC_ADDRESS"] = scanDevice->GetMac();
	dataValue["FIRMWARE_VERSION"] = scanDevice->GetVersionStr();
	dataValue["DEVICE_KEY"] = devKey;
	dataValue["NET_KEY"] = gateway->getBleNetKey();
	dataValue["APP_KEY"] = gateway->getBleAppKey();
	jsonValue["CMD"] = "NEW_DEVICE";
	jsonValue["DATA"] = dataValue;
#ifdef CONFIG_USE_OLD_APP
	PublishToLocalMessage(jsonValue);
#else
	PublishToDeviceTelemetry(jsonValue);
#endif

#ifdef CONFIG_USE_OLD_APP
	jsonValue["CMD"] = "NEW_CHILD_DEVICE";
	if (scanDevice->GetType() == BLE_SWITCH_RGB_2 || scanDevice->GetType() == BLE_SWITCH_RGB_2_SQUARE || scanDevice->GetType() == BLE_SWITCH_ELECTRICAL_2)
	{
		dataValue["PARENT_DEVICE_ID"] = scanDevice->GetId();
		dataValue["DEVICE_ID"] = Util::GenIdDeviceByElement(scanDevice->GetId(), 1);
		dataValue["DEVICE_UNICAST_ID"] = (int)scanDevice->GetAddr() + 1;
		dataValue["BUTTON_ID"] = 12;
		jsonValue["DATA"] = dataValue;
		PublishToLocalMessage(jsonValue);
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
			PublishToLocalMessage(jsonValue);
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
			PublishToLocalMessage(jsonValue);
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
			PublishToLocalMessage(jsonValue);
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
		device = new DeviceBleSwitchTouchRgb1(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_RGB_2:
	case BLE_SWITCH_RGB_2_SQUARE:
		device = new DeviceBleSwitchTouchRgb2(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_RGB_3:
	case BLE_SWITCH_RGB_3_SQUARE:
		device = new DeviceBleSwitchTouchRgb3(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_RGB_4:
	case BLE_SWITCH_RGB_4_SQUARE:
		device = new DeviceBleSwitchTouchRgb4(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_ELECTRICAL_1:
	case BLE_SWITCH_ELECTRICAL_WATER_HEATER:
		device = new DeviceBleSwitchElectrical1(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_ELECTRICAL_2:
		device = new DeviceBleSwitchElectrical2(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_ELECTRICAL_3:
		device = new DeviceBleSwitchElectrical3(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_ELECTRICAL_4:
		device = new DeviceBleSwitchElectrical4(id, name, mac, data, addr, type, version);
		break;
	case BLE_DC_SCENE_CONTACT:
	case BLE_REMOTE_M3:
	case BLE_REMOTE_M3_V2:
		device = new DeviceBleSwitchScene6DC(id, name, mac, data, addr, type, version);
		break;
	case BLE_AC_SCENE_CONTACT:
		device = new DeviceBleSwitchScene6AC(id, name, mac, data, addr, version);
		break;
	case BLE_AC_SCENE_CONTACT_RGB:
	case BLE_AC_SCENE_CONTACT_RGB_SQUARE:
		device = new DeviceBleSwitchScene6ACRgb(id, name, mac, data, addr, type, 1, version);
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
	case BLE_SWITCH_RGB_CURTAIN:
	case BLE_SWITCH_RGB_CURTAIN_SQUARE:
		device = new DeviceBleCurtain(id, name, mac, data, addr, type, version);
		break;
	case BLE_SWITCH_ROOLING_DOOR:
		device = new DeviceBleRoolDoor(id, name, mac, data, addr, type, version);
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
		{
			deviceList[id] = device;
#ifdef CONFIG_USE_OLD_APP
			Device *deviceChild = NULL;
			if (device->GetType() == BLE_SWITCH_RGB_2 || device->GetType() == BLE_SWITCH_RGB_2_SQUARE || device->GetType() == BLE_SWITCH_ELECTRICAL_2)
			{
				deviceChild = new DeviceBleSwitchTouchRgb1(Util::GenIdDeviceByElement(id, 1), name, mac, data, addr + 1, BLE_SWITCH_RGB_1, version);
				deviceList[Util::GenIdDeviceByElement(id, 1)] = deviceChild;
			}
			else if (device->GetType() == BLE_SWITCH_RGB_3 || device->GetType() == BLE_SWITCH_RGB_3_SQUARE || device->GetType() == BLE_SWITCH_ELECTRICAL_3)
			{
				for (int i = 1; i <= 2; i++)
				{
					deviceChild = new DeviceBleSwitchTouchRgb1(Util::GenIdDeviceByElement(id, i), name, mac, data, addr + i, BLE_SWITCH_RGB_1, version);
					deviceList[Util::GenIdDeviceByElement(id, i)] = deviceChild;
				}
			}
			else if (device->GetType() == BLE_SWITCH_RGB_4 || device->GetType() == BLE_SWITCH_RGB_4_SQUARE || device->GetType() == BLE_SWITCH_ELECTRICAL_4)
			{
				for (int i = 1; i <= 3; i++)
				{
					deviceChild = new DeviceBleSwitchTouchRgb1(Util::GenIdDeviceByElement(id, i), name, mac, data, addr + i, BLE_SWITCH_RGB_1, version);
					deviceList[Util::GenIdDeviceByElement(id, i)] = deviceChild;
				}
			}
			else if (device->GetType() == BLE_AC_SCENE_CONTACT_RGB || device->GetType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE)
			{
				for (int i = 1; i <= 5; i++)
				{
					deviceChild = new DeviceBleSwitchScene6ACRgb(Util::GenIdDeviceByElement(id, i), name, mac, data, addr, type, i + 1, version);
					deviceList[Util::GenIdDeviceByElement(id, i)] = deviceChild;
				}
			}
#endif
		}
		if (addDatabase)
		{
			database->DeviceAdd(device);
		}

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
		{
			groupListMtx.lock();
			groupList[group->GetId()] = group;
			groupListMtx.unlock();
		}
	}
	return group;
}

Rule *Gateway::AddRule(Json::Value &ruleValue, string name, bool addGateway, bool addDatabase)
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
			int logical = ruleValue["LOGICAL_OPERATOR_ID"].asInt();
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
				rule = new Rule(id, type, repeat, "", 0, Util::ConvertStrTimeToInt(startAt), Util::ConvertStrTimeToInt(endAt), ruleValue);
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
			rule = new Rule(id, type, repeat, "", 0, ruleValue);
			if (!rule)
			{
				LOGW("New rule error");
			}
		}
		else if (logical == -2)
		{
			type = "or";
			rule = new Rule(id, type, repeat, "", 0, ruleValue);
			if (!rule)
			{
				LOGW("New rule error");
			}
		}

		if (rule)
		{
			bool isEnable = (status) ? true : false;
			rule->SetStatus(isEnable);
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
								RuleOutputSceneBle *ruleOutputSceneBle = new RuleOutputSceneBle(sceneOutputRule, 0);
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
				ruleListMtx.lock();
				ruleList[id] = rule;
				ruleListMtx.unlock();
			}
			if (addDatabase)
			{
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
						Json::Value property = deviceOutput["PROPERTIES"];
						device = getDeviceFromId(devId);
						if (device)
						{
							SceneDelayDeviceOutput *sceneDelayDeviceOutput = new SceneDelayDeviceOutput(device, property, delay);
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
						Json::Value property = groupInSceneDelay["PROPERTIES"];
						group = getGroupFromId(groupId);
						if (group)
						{
							SceneDelayGroupOutput *sceneDelayGroupOutput = new SceneDelayGroupOutput(group, property, delay);
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
void Gateway::AddAllDeviceStatusV2(Json::Value &dataValue)
{
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		Json::Value deviceValue;
		deviceValue["id"] = device->GetId();
		Json::Value deviceAttbute;
		device->BuildTelemetryValueV2(deviceAttbute);
		deviceValue["data"] = deviceAttbute;
		dataValue.append(deviceValue);
	}
	deviceListMtx.unlock();
}

Rule *Gateway::AddRuleV2(Json::Value &ruleValue)
{
	// TODO: Check Rule id exist
	LOGD("OnAddRuleV2");
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
		if (!rule)
		{
			LOGE("New rule error, out of memory");
			return NULL;
		}

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
		return rule;
	}
	else
	{
		LOGW("Rule format error");
	}
	return NULL;
}

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
