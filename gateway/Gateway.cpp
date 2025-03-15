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
#include "TimerSchedule.h"
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
#include "DeviceBleRadaSensorAc.h"
#include "DeviceBleSmokeSensor.h"
#include "DeviceBleDoorSensor.h"
#include "DeviceBleScreenTouch.h"
#include "DeviceBleCurtain.h"
#include "DeviceBleRepeater.h"
#include "DeviceBleRoolDoor.h"
#include "DeviceBleSwitchTouch.h"
#include "DeviceBleSwitchCeiling.h"
#include "DeviceBleSeftPowerRemote.h"
#include "DeviceBleWifiSwitchTouch.h"
#include "DeviceBleWifiSwitchElectrical.h"
#include "DeviceBleSocketSwitch.h"
#include "DeviceBleWifiCurtain.h"
#include "DeviceBleWifiSwitchRoolDoor.h"
#include "DeviceBleSwitchKnob.h"
#include "DeviceBleLightAgriculturalSensor.h"
#include "DeviceBleTempHumAirAgriculturalSensor.h"
#include "DeviceBleTempHumSoilAgriculturalSensor.h"
#include "DeviceBlePhSoilAgriculturalSensor.h"
#include "DeviceBleEcTempHumSoilAgriculturalSensor.h"
#include "DeviceBlePhTempWaterAgriculturalSensor.h"
#include "DeviceBleOxyTempWaterAgriculturalSensor.h"
#include "DeviceBleEcSaliTdsWaterAgriculturalSensor.h"
#include "DeviceBleModuleInOut.h"
#include "DeviceBleLedHightBay.h"

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
#include "DeviceZigbeeLumiSensorMagnet.h"
#include "DeviceZigbeeLumiSensorSwitch.h"
#include "DeviceZigbeeLumiSensorTempHum.h"
#include "DeviceZigbeeLumiSensorWleakAQ1.h"
#include "DeviceZigbeeOnoff.h"

#include "DeviceZigbeeTuyaSensorMagnet.h"
#include "DeviceZigbeeTuyaSensorPir.h"
#include "DeviceZigbeeTuyaSensorHumanPresence.h"
#endif

#include "QrProtocol.h"
#include "RelayProtocol.h"

#define TIME_CHECK_OTA 10800

Gateway *gateway = NULL;

Gateway::Gateway(string mac, string address, int port, string clientId, string username, string password, int keepalive,
				 string localAddress, int localPort, string localUsername, string localPassword, int localKeepalive)
	: CloudProtocol(mac, address, port, clientId, username, password, keepalive, false),
	  LocalProtocol(mac, localAddress, localPort, mac, localUsername, localPassword, localKeepalive, false),
	  Udp(8181)
{
	this->mac = mac;
	this->id = "";
	this->dormitoryId = "";
	this->refresh_token = "";
	this->ble_addr = 0;
	this->ble_iv_index = 0;
	this->ble_netkey = "";
	this->ble_appkey = "";
	this->ble_devicekey = "";
	this->data = "";
	this->isAutoOta = true;
	this->lastTimePingGwBle = time(NULL);
}

Gateway::~Gateway()
{
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
	Device::InitDeviceModelList();
	database->GatewayRead();
	// LocalProtocol::init();
	CloudProtocol::init();
	// Udp::init();

	InitUdpMessage();
	InitMqttMessageDevice();
	InitMqttMessageGroup();
	InitMqttMessageRoom();
	InitMqttMessageRule();
	InitMqttMessageScene();
	InitMqttMessageHc();

#ifdef ESP_PLATFORM
	LOGI("Free memory: %d bytes, internal: %d bytes", esp_get_free_heap_size(), esp_get_free_internal_heap_size());
	// if (xTaskCreate(startUdpThread, "Udp", 5120, this, 7, NULL) != pdPASS)
	// {
	// 	LOGE("Failed to create task");
	// 	SetLedService(false);
	// }
	// vTaskDelay(10);
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
	thread testSwitchThread(bind(&Gateway::TestSwitch, this));
	testSwitchThread.detach();
#endif

	database->DeviceRead();
	database->DeviceBleChildRead();
#ifdef CONFIG_SAVE_ATTRIBUTE
	database->DeviceAttributeRead();
#endif
	database->RoomRead();
	database->GroupRead();
	database->DeviceInGroupRead();
	database->SceneBleRead();
	database->DeviceInSceneBleRead();
	database->DeviceInRoomRead();
	database->RuleRead();
#ifdef __ANDROID__
	database->NotiRead();
#endif
	if (gateway->getId().compare("") == 0)
	{
		id = mac;
		gateway->setId(id);
		database->GatewayAdd(gateway);
		database->GatewayRead();
	}

	string firmwareVer = STR(VERSION);
	if (getVersion() != firmwareVer)
	{
		setVersion(firmwareVer);
		database->GatewayUpdateVersion(this, firmwareVer);
	}

	// LocalConnect();
	int cloudConnected = CloudConnect();

#ifdef __ANDROID__
	if (cloudConnected != MQTT_ERR_SUCCESS)
	{
		Json::Value jsonValue;
		Json::Value dataValue;
		dataValue["status"] = 0;
		dataValue["version"] = STR(VERSION);
		dataValue["ip"] = Wifi::GetIP();
		jsonValue["cmd"] = "homeController";
		jsonValue["data"] = dataValue;
		LocalPublish(jsonValue);
	}
#endif

	// Get data isAutoOta
	Json::Value dataJson;
	dataJson.parse(gateway->getData());
	if (dataJson.isObject() && dataJson.isMember("isAutoOta") && dataJson["isAutoOta"].isBool())
	{
		this->isAutoOta = dataJson["isAutoOta"].asBool();
	}

	if (timerSchedule) // Pre-instantiated timerSchedule for gateway
	{
		timerSchedule->RegisterTimer(TIME_CHECK_OTA, std::bind(&Gateway::CheckAutoOta, this));
	}
}

void Gateway::OnCloudConnect(bool isConnected, bool isReconnect)
{
	LOGI("OnCloudConnect: %d", isConnected);
	Json::Value jsonValue;
	Json::Value dataValue;
	dataValue["status"] = isConnected ? 1 : 0;
	dataValue["version"] = STR(VERSION);
	dataValue["ip"] = Wifi::GetIP();
	jsonValue["cmd"] = "homeController";
	jsonValue["data"] = dataValue;
	LocalPublish(jsonValue);

	if (isConnected)
	{
		Util::LedInternet(true);
		OnlineHC(mac);
		GatewayConnectToCloudNotice();
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

	if (database->IsHaveDb(DB_NAME_V1))
	{
		if (unlink(DB_NAME_V1) != 0)
		{
			LOGE("Failed to delete file db_v1\n");
		}
	}

	if (database->IsHaveDb(DB_NAME "1"))
	{
		if (unlink(DB_NAME "1") != 0)
		{
			LOGE("Failed to delete file db_restore\n");
		}
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
		// bleProtocol->ResetFactory();
	}
	else
		LOGW("BleProtocol null");

	DelDatabase();
}

void Gateway::CheckAutoOta()
{
	if (this->getAutoOta())
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
#elif defined(__ANDROID__)
		data["data"]["type"] = 3;
#endif
		CloudPublish(data);
	}
}

int Gateway::RestartBleGw()
{
	LOGW("Restart Gateway Ble");
#ifdef __ANDROID__
	Util::ExecuteCMD("su");
	Util::ExecuteCMD("echo 100 > /sys/class/gpio/export");
	Util::ExecuteCMD("echo out > /sys/class/gpio/gpio100/direction");
	Util::ExecuteCMD("echo 1 > /sys/class/gpio/gpio100/value");
	sleep(1);
	Util::ExecuteCMD("echo 0 > /sys/class/gpio/gpio100/value");
	Util::ExecuteCMD("echo 100 > /sys/class/gpio/unexport");
#elif defined(__OPENWRT__)
	Util::ExecuteCMD("echo '0' > /sys/class/gpio/gpio1/value");
	sleep(1);
	Util::ExecuteCMD("echo '1' > /sys/class/gpio/gpio1/value");
#elif defined(ESP_PLATFORM)
	SetGpioResetGwBle();
#endif
	return CODE_OK;
}

static string ST_array_icon[18] = {"01d", "02d", "03d", "04d", "09d", "10d", "11d", "13d", "50d", "01n", "02n", "03n", "04n", "09n", "10n", "11n", "13n", "50n"};

int Gateway::CheckOnlineThread()
{
	LOGI("Start CheckOnlineThread");
	time_t currentTime = 0;
	time_t oldTime = 0;
	time_t oldTimeCheckStatus = 0;
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

	Json::Value devicesStatusOld = Json::Value::null;

	while (1)
	{
		if ((time(NULL) - oldTime) > 1800)
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

		if (!bleProtocol->IsProvision() && !LocalProtocol::IsBusy() && !CloudProtocol::IsBusy() && bleProtocol->isInitKey)
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

			if (allTimeCheck > 0)
			{
				currentTime = time(NULL);
				// LOGE("current: %ld", currentTime);
				// LOGE("lastime: %ld", getLastTimePingGwBle());
				// LOGE("alltime: %d", allTimeCheck);
				if (currentTime - getLastTimePingGwBle() > allTimeCheck)
				{
					setLastTimePingGwBle(time(NULL));
					RestartBleGw();
				}
			}
		}
		sleep(1);
	}
	return CODE_OK;
}

int Gateway::TestSwitch()
{
	uint32_t timeout;
	bool checkRssi;
	bool checkRelay1On;
	bool checkRelay2On;
	bool checkRelay3On;
	bool checkRelay4On;
	bool checkRelay1Off;
	bool checkRelay2Off;
	bool checkRelay3Off;
	bool checkRelay4Off;
	bool checkOnAll;
	bool checkOffAll;
	while (1)
	{
		if (qrProtocol->startTest)
		{
			checkRssi = false;
			checkRelay1On = false;
			checkRelay2On = false;
			checkRelay3On = false;
			checkRelay4On = false;
			checkRelay1Off = false;
			checkRelay2Off = false;
			checkRelay3Off = false;
			checkRelay4Off = false;
			checkOnAll = false;
			checkOffAll = false;

			bleProtocol->StartScan();
			bleProtocol->isMatchMac = false;
			timeout = 3000;
			while (!bleProtocol->isMatchMac && timeout--)
			{
				usleep(1000);
			}
			bleProtocol->StopScan();
			if (bleProtocol->isMatchMac)
			{
				checkRssi = true;
			}
			bleProtocol->SetOnOffLight(65535, 0, 5, true);
			if ((bleProtocol->SetOnOffLight(qrProtocol->addr, 1, 5, true) == CODE_OK))
			{
				sleep(2);
				if (relayProtocol->rl1 == 1)
					checkRelay1On = true;
			}
			if ((bleProtocol->SetOnOffLight(qrProtocol->addr + 1, 1, 5, true) == CODE_OK))
			{
				sleep(2);
				if (relayProtocol->rl2 == 1)
					checkRelay2On = true;
			}
			if ((bleProtocol->SetOnOffLight(qrProtocol->addr + 2, 1, 5, true) == CODE_OK))
			{
				sleep(2);
				if (relayProtocol->rl3 == 1)
					checkRelay3On = true;
			}
			if ((bleProtocol->SetOnOffLight(qrProtocol->addr + 3, 1, 5, true) == CODE_OK))
			{
				sleep(2);
				if (relayProtocol->rl4 == 1)
					checkRelay4On = true;
			}

			if ((bleProtocol->SetOnOffLight(qrProtocol->addr, 0, 5, true) == CODE_OK))
			{
				sleep(2);
				if (relayProtocol->rl1 == 0)
					checkRelay1Off = true;
			}
			if ((bleProtocol->SetOnOffLight(qrProtocol->addr + 1, 0, 5, true) == CODE_OK))
			{
				sleep(2);
				if (relayProtocol->rl2 == 0)
					checkRelay2Off = true;
			}
			if ((bleProtocol->SetOnOffLight(qrProtocol->addr + 2, 0, 5, true) == CODE_OK))
			{
				sleep(2);
				if (relayProtocol->rl3 == 0)
					checkRelay3Off = true;
			}
			if ((bleProtocol->SetOnOffLight(qrProtocol->addr + 3, 0, 5, true) == CODE_OK))
			{
				sleep(2);
				if (relayProtocol->rl4 == 0)
					checkRelay4Off = true;
			}

			bleProtocol->SetOnOffLight(65535, 1, 5, true);
			sleep(2);
			if (relayProtocol->rl1 && relayProtocol->rl2 && relayProtocol->rl3 && relayProtocol->rl4)
				checkOnAll = true;
			bleProtocol->SetOnOffLight(65535, 0, 5, true);
			sleep(2);
			if (!relayProtocol->rl1 && !relayProtocol->rl2 && !relayProtocol->rl3 && !relayProtocol->rl4)
				checkOffAll = true;
			Json::Value rs;
			rs["mac"] = qrProtocol->mac;
			rs["addr"] = qrProtocol->addr;
			rs["rssi"] = checkRssi ? bleProtocol->rssi : 0;
			rs["on_relay1"] = checkRelay1On;
			rs["on_relay2"] = checkRelay2On;
			rs["on_relay3"] = checkRelay3On;
			rs["on_relay4"] = checkRelay4On;
			rs["off_relay1"] = checkRelay1Off;
			rs["off_relay2"] = checkRelay2Off;
			rs["off_relay3"] = checkRelay3Off;
			rs["off_relay4"] = checkRelay4Off;
			rs["on_all"] = checkOnAll;
			rs["off_all"] = checkOffAll;
			Json::Value deviceJson = Json::arrayValue;
			deviceJson.append(rs);

			Json::Value dataPush;
			dataPush["cmd"] = "hcReportLog",
			dataPush["rpi"] = Util::genRandRQI(16),
			dataPush["device"] = deviceJson;
			LOGE("%s", dataPush.toString().c_str());
			this->CloudPublish(dataPush.toString());
			qrProtocol->startTest = false;
		}
		sleep(2);
	}
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
			SLEEP_MS(500);
			send(dataStr, &s, sizeof(s));
			Util::LedInternet(true);
			SLEEP_MS(500);
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

	return CloudPublish(respValue.toString());
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

	Json::Value devValue;
	devValue["id"] = scanDevice->GetId();
	devValue["addr"] = scanDevice->GetAddr();
	devValue["ver"] = scanDevice->GetVersionStr();
	devValue["mac"] = scanDevice->GetMac();
	devValue["data"] = scanDevice->GetData();
	if (scanDevice->GetType() == ZIGBEE_LUMI_PLUG ||
		scanDevice->GetType() == ZIGBEE_LUMI_SENSOR_SWITCH)
	{
		devValue["type"] = BLE_SWITCH_ONOFF;
	}
	else if (scanDevice->GetType() == ZIGBEE_LUMI_SENSOR_TEMP_HUM)
	{
		devValue["type"] = BLE_TEMP_HUM_SENSOR;
	}
	else if (scanDevice->GetType() == ZIGBEE_LUMI_SENSOR_WLEAK_AQ1)
	{
		devValue["type"] = BLE_SMOKE_SENSOR;
	}
	else if (scanDevice->GetType() == ZIGBEE_LUMI_SENSOR_MAGNET ||
			 scanDevice->GetType() == ZIGBEE_TUYA_SENSOR_MAGNET_TY0203)
	{
		devValue["type"] = BLE_DOOR_SENSOR;
	}
	else if (scanDevice->GetType() == ZIGBEE_TUYA_SENSOR_PIR_RH3040 ||
			 scanDevice->GetType() == ZIGBEE_TUYA_SENSOR_HUMAN_PRESENCE_TS0225)
	{
		devValue["type"] = BLE_PIR_LIGHT_SENSOR_DC;
	}
	else
	{
		devValue["type"] = (Json::Value::UInt)scanDevice->GetType();
	}
	jsonValue["device"].append(devValue);
	pushNewDeviceLocal(jsonValue);
}

Device *Gateway::AddNewDevice(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, bool addDatabase)
{
	LOGI("Add new device id: %s, name: %s, mac: %s, addr: 0x%04X, type: 0x%04X, verion: %d", id.c_str(), name.c_str(), mac.c_str(), addr, type, version);
	Device *device = getDeviceFromId(id);
	if (device)
	{
		bleProtocol->ResetDev(device->GetAddr());
	}

	switch (type)
	{
	case BLE_ALL:
		device = new DeviceBleAll(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_LED_CHIEU_TRANH:
	case BLE_LED_CHIEU_GUONG:
	case BLE_DEN_BAN:
	case BLE_DOWNLIGHT_SMT:
	case BLE_DOWNLIGHT_COB_GOC_HEP:
	case BLE_DOWNLIGHT_COB_GOC_RONG:
	case BLE_DOWNLIGHT_COB_TRANG_TRI:
	case BLE_LED_FLOOD:
	case BLE_LED_AT39:
	case BLE_LED_AT40:
	case BLE_LED_AT41:
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
		device = new DeviceBleLightOnoffCctDim(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_DOWNLIGHT_RGBCW:
	case BLE_LED_DAY_RGBCW:
	case BLE_LED_BULB:
		device = new DeviceBleLightOnoffCctDimHslModeRGB(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_LED_DAY_RGB:
		device = new DeviceBleLightOnoffHslModeRGB(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_LED_HIGHTBAY:
		device = new DeviceBleLedHightBay(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_SWITCH_ONOFF:
	case BLE_SWITCH_ONOFF_V2:
		device = new DeviceBleSwitchOnoff(id, name, mac, dataJson, addr, type, version);
		break;

	case BLE_SWITCH_RGB_SOCKET_1:
	case BLE_SWITCH_RGB_SOCKET_1_V2:
		device = new DeviceBleSocketSwitch(id, name, mac, dataJson, addr, type, version, 1);
		break;
	case BLE_SWITCH_RGB_SOCKET_2:
		device = new DeviceBleSocketSwitch(id, name, mac, dataJson, addr, type, version, 2);
		break;
	case BLE_SWITCH_RGB_1:
	case BLE_SWITCH_RGB_1_SQUARE:
	case BLE_SWITCH_RGB_WATER_HEATER:
	case BLE_SWITCH_RGB_1_V2:
	case BLE_SWITCH_RGB_1_SQUARE_V2:
		device = new DeviceBleSwitchTouchRgb(id, name, mac, dataJson, addr, type, version, 1);
		break;
	case BLE_SWITCH_RGB_2:
	case BLE_SWITCH_RGB_2_SQUARE:
	case BLE_SWITCH_RGB_2_V2:
	case BLE_SWITCH_RGB_2_SQUARE_V2:
		device = new DeviceBleSwitchTouchRgb(id, name, mac, dataJson, addr, type, version, 2);
		break;
	case BLE_SWITCH_RGB_3:
	case BLE_SWITCH_RGB_3_SQUARE:
	case BLE_SWITCH_RGB_3_V2:
	case BLE_SWITCH_RGB_3_SQUARE_V2:
		device = new DeviceBleSwitchTouchRgb(id, name, mac, dataJson, addr, type, version, 3);
		break;
	case BLE_SWITCH_RGB_4:
	case BLE_SWITCH_RGB_4_SQUARE:
	case BLE_SWITCH_RGB_4_V2:
	case BLE_SWITCH_RGB_4_SQUARE_V2:
		device = new DeviceBleSwitchTouchRgb(id, name, mac, dataJson, addr, type, version, 4);
		break;
	case BLE_SWITCH_ELECTRICAL_1:
	case BLE_SWITCH_ELECTRICAL_1_V2:
	case BLE_SWITCH_ELECTRICAL_WATER_HEATER:
		device = new DeviceBleSwitchElectrical(id, name, mac, dataJson, addr, type, version, 1);
		break;
	case BLE_SWITCH_ELECTRICAL_2:
	case BLE_SWITCH_ELECTRICAL_2_V2:
		device = new DeviceBleSwitchElectrical(id, name, mac, dataJson, addr, type, version, 2);
		break;
	case BLE_SWITCH_ELECTRICAL_3:
	case BLE_SWITCH_ELECTRICAL_3_V2:
		device = new DeviceBleSwitchElectrical(id, name, mac, dataJson, addr, type, version, 3);
		break;
	case BLE_SWITCH_ELECTRICAL_4:
		device = new DeviceBleSwitchElectrical(id, name, mac, dataJson, addr, type, version, 4);
		break;
	case BLE_WIFI_SWITCH_ELECTRICAL_1:
		device = new DeviceBleWifiSwitchElectrical(id, name, mac, dataJson, addr, type, version, 1);
		break;
	case BLE_WIFI_SWITCH_ELECTRICAL_2:
		device = new DeviceBleWifiSwitchElectrical(id, name, mac, dataJson, addr, type, version, 2);
		break;
	case BLE_WIFI_SWITCH_ELECTRICAL_3:
		device = new DeviceBleWifiSwitchElectrical(id, name, mac, dataJson, addr, type, version, 3);
		break;
	case BLE_SWITCH_2_CEILING:
		device = new DeviceBleSwitchCeiling(id, name, mac, dataJson, addr, type, version, 2);
		break;
	case BLE_SWITCH_3_CEILING:
		device = new DeviceBleSwitchCeiling(id, name, mac, dataJson, addr, type, version, 3);
		break;
	case BLE_SWITCH_5_CEILING:
		device = new DeviceBleSwitchCeiling(id, name, mac, dataJson, addr, type, version, 5);
		break;
	case BLE_DC_SCENE_CONTACT:
	case BLE_REMOTE_M3:
	case BLE_REMOTE_M3_V2:
	case BLE_REMOTE_M4:
		device = new DeviceBleSwitchScene6DC(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_AC_SCENE_CONTACT:
		device = new DeviceBleSwitchScene6AC(id, name, mac, dataJson, addr, version);
		break;
	case BLE_AC_SCENE_CONTACT_RGB:
	case BLE_AC_SCENE_CONTACT_RGB_SQUARE:
		device = new DeviceBleSwitchScene6ACRgb(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_TEMP_HUM_SENSOR:
		device = new DeviceBleSensorTempHum(id, name, mac, dataJson, addr, version);
		break;
	case BLE_PM_SENSOR:
		device = new DeviceBleSensorPm(id, name, mac, dataJson, addr, version);
		break;
	case BLE_PIR_LIGHT_SENSOR_DC:
	case BLE_PIR_LIGHT_SENSOR_DC_CB10:
	case BLE_PIR_LIGHT_SENSOR_DC_CB09:
		device = new DeviceBlePirLightSensorDC(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_RADA_LIGHT_SENSOR_AC_CB15:
		device = new DeviceBleRadaSensorAc(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_PIR_LIGHT_SENSOR_AC:
		device = new DeviceBlePirLightSensorAC(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_PIR_LIGHT_SENSOR_AC_AMTRAN:
		device = new DeviceBlePirLightSensorAC_CB09(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_SMOKE_SENSOR:
		device = new DeviceBleSmokeSensor(id, name, mac, dataJson, addr, version);
		break;
	case BLE_DOOR_SENSOR:
	case BLE_DOOR_CB16_SENSOR:
		device = new DeviceBleDoorSensor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_AC_SCENE_SCREEN_TOUCH:
		device = new DeviceBleScreenTouch(id, name, mac, dataJson, addr, version);
		break;
	case BLE_SWITCH_CURTAIN:
	case BLE_SWITCH_RGB_CURTAIN:
	case BLE_SWITCH_RGB_CURTAIN_SQUARE:
	case BLE_SWITCH_RGB_CURTAIN_HCN:
	case BLE_SWITCH_RGB_CURTAIN_SQUARE_V2:
		device = new DeviceBleCurtain(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_SWITCH_ROOLING_DOOR:
	case BLE_SWITCH_ROOLING_DOOR_V2:
	case BLE_SWITCH_ROOLING_DOOR_SQUARE:
		device = new DeviceBleRoolDoor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_SWITCH_1:
	case BLE_SWITCH_WATER_HEATER:
		device = new DeviceBleSwitchTouch(id, name, mac, dataJson, addr, type, version, 1);
		break;
	case BLE_SWITCH_2:
		device = new DeviceBleSwitchTouch(id, name, mac, dataJson, addr, type, version, 2);
		break;
	case BLE_SWITCH_3:
		device = new DeviceBleSwitchTouch(id, name, mac, dataJson, addr, type, version, 3);
		break;
	case BLE_SWITCH_4:
		device = new DeviceBleSwitchTouch(id, name, mac, dataJson, addr, type, version, 4);
		break;
	case BLE_REPEATER:
		device = new DeviceBleRepeater(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_WIFI_SWITCH_1:
	case BLE_WIFI_SWITCH_1_SQUARE:
		device = new DeviceBleWifiSwitchTouch(id, name, mac, dataJson, addr, type, version, 1);
		break;
	case BLE_WIFI_SWITCH_2:
	case BLE_WIFI_SWITCH_2_SQUARE:
		device = new DeviceBleWifiSwitchTouch(id, name, mac, dataJson, addr, type, version, 2);
		break;
	case BLE_WIFI_SWITCH_3:
	case BLE_WIFI_SWITCH_3_SQUARE:
		device = new DeviceBleWifiSwitchTouch(id, name, mac, dataJson, addr, type, version, 3);
		break;
	case BLE_WIFI_SWITCH_4:
	case BLE_WIFI_SWITCH_4_SQUARE:
		device = new DeviceBleWifiSwitchTouch(id, name, mac, dataJson, addr, type, version, 4);
		break;
	case BLE_WIFI_SWITCH_ROOLING_DOOR:
	case BLE_WIFI_SWITCH_ROOLING_DOOR_SQUARE:
		device = new DeviceBleWifiSwitchRoolDoor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_WIFI_SWITCH_CURTAIN:
	case BLE_WIFI_SWITCH_CURTAIN_SQUARE:
		device = new DeviceBleWifiCurtain(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_SEFTPOWER_REMOTE_1:
	case BLE_SEFTPOWER_REMOTE_2:
	case BLE_SEFTPOWER_REMOTE_3:
	case BLE_SEFTPOWER_REMOTE_6:
		device = getDeviceFromId(id);
		if (device)
		{
			DeviceBleSeftPowerRemote *deviceBleSeftPowerRemote = dynamic_cast<DeviceBleSeftPowerRemote *>(device);
			if (deviceBleSeftPowerRemote)
			{
				Device *parent = deviceBleSeftPowerRemote->GetParent();
				if (parent)
				{
					database->DeviceBleChildDel(deviceBleSeftPowerRemote, parent);
					bleProtocol->ResetSeftPowerRemote(parent->GetAddr(), deviceBleSeftPowerRemote->GetAddr());
				}
				else
					LOGW("parent device null");
			}
			delDevice(device);
		}
		device = new DeviceBleSeftPowerRemote(id, name, mac, dataJson, addr, type, version, NULL);
		break;
	case BLE_SWITCH_KNOB:
		device = new DeviceBleSwitchKnob(id, name, mac, dataJson, addr, type, version, 2);
		break;
	case BLE_TEMP_HUM_AIR_AGRICULTURAL_SENSOR:
		device = new DeviceBleTempHumAirAgriculturalSensor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_LIGHT_SENSOR:
	case BLE_LIGHT_SENSOR_AGRI:
		device = new DeviceBleLightAgriculturalSensor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_TEMP_HUM_SOIL_AGRICULTURAL_SENSOR:
		device = new DeviceBleTempHumSoilAgriculturalSensor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_PH_SOIL_AGRICULTURAL_SENSOR:
		device = new DeviceBlePhSoilAgriculturalSensor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_EC_TEMP_HUM_SOIL_AGRICULTURAL_SENSOR:
		device = new DeviceBleEcTempHumSoilAgriculturalSensor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_PH_WATER_AGRICULTURAL_SENSOR:
		device = new DeviceBlePhTempWaterAgriculturalSensor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_EC_WATER_AGRICULTURAL_SENSOR:
		device = new DeviceBleEcSaliTdsWaterAgriculturalSensor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_OXY_WATER_AGRICULTURAL_SENSOR:
		device = new DeviceBleOxyTempWaterAgriculturalSensor(id, name, mac, dataJson, addr, type, version);
		break;
	case BLE_MODULE_INOUT:
		device = new DeviceBleModuleInOut(id, name, mac, dataJson, addr, type, version, 2, 4);
		break;

#ifndef ESP_PLATFORM
	case MQTT_AI_HUB:
		device = new DeviceMqttAihub(id, name, mac, dataJson, version);
		break;
	case CAMERA_TUYA:
	case CAMERA_DAHUA:
	case CAMERA_HKVISION:
		device = new Device(id, name, mac, dataJson, addr, type, version);
		break;
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
	case ZIGBEE_LUMI_SENSOR_MAGNET:
		device = new DeviceZigbeeLumiSensorMagnet(id, name, mac, dataJson, addr);
		break;
	case ZIGBEE_LUMI_SENSOR_SWITCH:
		device = new DeviceZigbeeLumiSensorSwitch(id, name, mac, dataJson, addr);
		break;
	case ZIGBEE_LUMI_SENSOR_TEMP_HUM:
		device = new DeviceZigbeeLumiSensorTempHum(id, name, mac, dataJson, addr);
		break;
	case ZIGBEE_LUMI_SENSOR_WLEAK_AQ1:
		device = new DeviceZigbeeLumiSensorWleakAQ1(id, name, mac, dataJson, addr);
		break;
	case ZIGBEE_LUMI_PLUG:
		device = new DeviceZigbeeOnoff(id, name, mac, dataJson, addr);
		break;
	case ZIGBEE_TUYA_SENSOR_MAGNET_TY0203:
		device = new DeviceZigbeeTuyaSensorMagnet(id, name, mac, dataJson, addr);
		break;
	case ZIGBEE_TUYA_SENSOR_PIR_RH3040:
		device = new DeviceZigbeeTuyaSensorPir(id, name, mac, dataJson, addr);
		break;
	case ZIGBEE_TUYA_SENSOR_HUMAN_PRESENCE_TS0225:
		device = new DeviceZigbeeTuyaSensorHumanPresence(id, name, mac, dataJson, addr);
		break;
#endif

	default:
		LOGW("Add new device not support type: 0x%04X", type);
		break;
	}

	if (device)
	{
		deviceList[id] = device;
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

Group *Gateway::AddNewGroup(Group *group, bool addDatabase)
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
		groupListMtx.lock();
		groupList[group->GetId()] = group;
		groupListMtx.unlock();
	}
	return group;
}

Rule *Gateway::AddRule(Json::Value &ruleValue, bool addDatabase)
{
	LOGD("OnAddRule");
	if (ruleValue.isMember("id") && ruleValue["id"].isString() &&
		ruleValue.isMember("name") && ruleValue["name"].isString() &&
		ruleValue.isMember("type") && ruleValue["type"].isInt() &&
		ruleValue.isMember("input") && ruleValue["input"].isObject() &&
		ruleValue.isMember("output") && ruleValue["output"].isArray())
	{
		string id = ruleValue["id"].asString();
		int type = ruleValue["type"].asInt();
		Json::Value inputValue = ruleValue["input"];
		Json::Value outputValues = ruleValue["output"];
		string name = ruleValue["name"].asString();
		int repeat = 255;
		uint16_t addr = 0;
		Rule *rule = NULL;

		if (inputValue.isMember("timer") && inputValue["timer"].isObject())
		{
			Json::Value timer = inputValue["timer"];
			if (timer.isMember("start") && timer["start"].isString())
			{
				repeat = 0;
				if (inputValue.isMember("repeat") && inputValue["repeat"].isInt())
				{
					repeat = inputValue["repeat"].asInt();
				}
				string startAt = timer["start"].asString();
				string endAt = "";
				if (timer.isMember("end") && timer["end"].isString())
				{
					endAt = timer["end"].asString();
				}
				int startAtInt = Util::ConvertStrTimeToInt(startAt);
				int endAtInt = Util::ConvertStrTimeToInt(endAt);
				if ((startAtInt == endAtInt) || (startAtInt > 0 && endAtInt < 0))
				{
					rule = new Rule(id, (RuleType)type, repeat, name, addr, ruleValue);
					RuleInputTimer *ruleInputTimer = new RuleInputTimer(rule, startAtInt, repeat);
					rule->AddRuleInput(ruleInputTimer);
				}
				else
					rule = new Rule(id, (RuleType)type, repeat, name, addr, startAtInt, endAtInt, ruleValue);
			}
		}
		else
		{
			rule = new Rule(id, (RuleType)type, repeat, name, addr, ruleValue);
		}

		if (rule)
		{
			if (ruleValue.isMember("enable") && ruleValue["enable"].isInt())
				rule->SetStatus(ruleValue["enable"].asInt());
			// if (ruleValue.isMember("isFirstRun") && ruleValue["isFirstRun"].isBool())
			// {
			// 	rule->SetFirstRun(ruleValue["isFirstRun"].asBool());
			// }

			if (inputValue.isMember("device") && inputValue["device"].isArray())
			{
				Json::Value devicesJson = inputValue["device"];
				for (auto &deviceJson : devicesJson)
				{
					if (deviceJson.isObject() && deviceJson.isMember("id") && deviceJson["id"].isString() &&
						deviceJson.isMember("data") && deviceJson["data"].isObject())
					{
						string deviceId = deviceJson["id"].asString();
						Device *deviceInRule = getDeviceFromId(deviceId);
						if (deviceInRule)
						{
							Json::Value dataJson = deviceJson["data"];
							RuleInputDevice *ruleInputDevice = new RuleInputDevice(rule, deviceInRule, dataJson);
							rule->AddRuleInput(ruleInputDevice);
						}
						else
							LOGW("Device not found");
					}
				}
			}

			for (Json::Value::ArrayIndex i = 0; i < outputValues.size(); i++)
			{
				Json::Value outputValue = outputValues[i];
				if (outputValue.isMember("delay"))
				{
					RuleOutputDelay *ruleOutputDelay = new RuleOutputDelay(outputValue["delay"].asInt());
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
						else
							LOGW("Device not found");
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
			ruleListMtx.lock();
			ruleList[id] = rule;
			ruleListMtx.unlock();
			if (addDatabase)
			{
				string ruleStr = ruleValue.toString();
				// ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->RuleAdd(rule, ruleStr, 0);
				bool isFirstStt = false;
				if (repeat == 0)
				{
					isFirstStt = true;
				}
				rule->SetFirstRun(isFirstStt);
				database->RuleUpdateFirstRun(rule, isFirstStt);
			}

			if (ruleValue.isMember("roomId") && ruleValue["roomId"].isString())
			{
				string roomId = ruleValue["roomId"].asString();
				Room *room = getRoomFromId(roomId);
				if (room)
				{
					room->AddRule(rule, true, true);
				}
			}
		}
		else
			LOGW("Rule null");
		return rule;
	}
	else
		LOGE("New rule error, out of memory");
	return NULL;
}

Rule *Gateway::AddRuleV2(Json::Value &ruleValue, bool addDatabase)
{
	// TODO: Check Rule id exist
	LOGD("OnAddRuleV2");
	if (ruleValue.isMember("id") && ruleValue["id"].isString() &&
		ruleValue.isMember("name") && ruleValue["name"].isString() &&
		ruleValue.isMember("type") && ruleValue["type"].isInt() &&
		ruleValue.isMember("input") && ruleValue["input"].isObject() &&
		ruleValue.isMember("output") && ruleValue["output"].isArray())
	{
		string id = ruleValue["id"].asString();
		int type = ruleValue["type"].asInt();
		Json::Value inputValue = ruleValue["input"];
		Json::Value outputValues = ruleValue["output"];
		string name = ruleValue["name"].asString();
		uint16_t addr = 0;
		Rule *rule = NULL;
		bool isFullDay = true;
		int repeat = 255;

		if (ruleValue.isMember("time") && ruleValue["time"].isObject())
		{
			Json::Value timeRule = ruleValue["time"];
			if (timeRule.isMember("fullDay") && timeRule["fullDay"].isBool())
			{
				isFullDay = timeRule["fullDay"].asBool();
				if (isFullDay)
				{
					rule = new Rule(id, (RuleType)type, repeat, name, addr, ruleValue);
				}
				else
				{
					if (timeRule.isMember("repeat") && timeRule["repeat"].isInt() &&
						timeRule.isMember("start") && timeRule["start"].isString() &&
						timeRule.isMember("end") && timeRule["end"].isString())
					{
						repeat = timeRule["repeat"].asInt();
						string startRule = timeRule["start"].asString();
						string endRule = timeRule["end"].asString();
						rule = new Rule(id, (RuleType)type, repeat, name, addr, Util::ConvertStrTimeToInt(startRule), Util::ConvertStrTimeToInt(endRule), ruleValue);
					}
					else
					{
						LOGW("Time format error");
						return NULL;
					}
				}
			}
		}

		if (rule)
		{
			if (ruleValue.isMember("enable") && ruleValue["enable"].isInt())
				rule->SetStatus(ruleValue["enable"].asInt());

			// if (ruleValue.isMember("isFirstRun") && ruleValue["isFirstRun"].isBool())
			// 	rule->SetFirstRun(ruleValue["isFirstRun"].asBool());

			// parse input
			if (inputValue.isMember("timer") && inputValue["timer"].isArray())
			{
				Json::Value timersJson = inputValue["timer"];
				for (auto &timerJson : timersJson)
				{
					if (timerJson.isMember("time") && timerJson["time"].isString() &&
						timerJson.isMember("repeat") && timerJson["repeat"].isInt())
					{
						string timerTime = timerJson["time"].asString();
						int timerRepeat = timerJson["repeat"].asInt();
						RuleInputTimer *ruleInputTimer = new RuleInputTimer(rule, Util::ConvertStrTimeToInt(timerTime), timerRepeat);
						rule->AddRuleInput(ruleInputTimer);
					}
				}
			}

			if (inputValue.isMember("device") && inputValue["device"].isArray())
			{
				Json::Value devicesJson = inputValue["device"];
				for (auto &deviceJson : devicesJson)
				{
					if (deviceJson.isObject() && deviceJson.isMember("id") && deviceJson["id"].isString() &&
						deviceJson.isMember("data") && deviceJson["data"].isObject())
					{
						string deviceId = deviceJson["id"].asString();
						Device *deviceInRule = getDeviceFromId(deviceId);
						if (deviceInRule)
						{
							Json::Value dataJson = deviceJson["data"];
							RuleInputDevice *ruleInputDevice = new RuleInputDevice(rule, deviceInRule, dataJson);
							rule->AddRuleInput(ruleInputDevice);
						}
						else
							LOGW("Device not found");
					}
				}
			}

			// parse output
			for (Json::Value::ArrayIndex i = 0; i < outputValues.size(); i++)
			{
				Json::Value outputValue = outputValues[i];
				if (outputValue.isMember("time") && outputValue["time"].isInt())
				{
					RuleOutputDelay *ruleOutputDelay = new RuleOutputDelay(outputValue["time"].asInt());
					rule->AddRuleOutput(ruleOutputDelay);
				}
				else if (outputValue.isMember("devId"))
				{
					if (outputValue["devId"].isString() && outputValue.isMember("data") && outputValue["data"].isObject())
					{
						Json::Value dataValue = outputValue["data"];
						string id = outputValue["devId"].asString();
						Device *device = gateway->getDeviceFromId(id);
						if (device)
						{
							RuleOutputDevice *ruleOutputDevice = new RuleOutputDevice(device, dataValue);
							rule->AddRuleOutput(ruleOutputDevice);
						}
						else
							LOGW("Device not found");
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
			ruleListMtx.lock();
			ruleList[id] = rule;
			ruleListMtx.unlock();
			if (addDatabase)
			{
				ruleValue["isFirstRun"] = true;
				string ruleStr = ruleValue.toString();
				// ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->RuleAdd(rule, ruleStr, 0);

				bool isFirstStt = false;
				if (repeat == 0)
				{
					isFirstStt = true;
				}
				rule->SetFirstRun(isFirstStt);
				database->RuleUpdateFirstRun(rule, isFirstStt);
			}
		}
		else
			LOGW("Rule null");
		return rule;
	}
	else
		LOGE("New rule error, out of memory");
	return NULL;
}

SceneBle *Gateway::AddNewSceneBle(SceneBle *sceneBle, bool addDatabase)
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
		sceneBleListMtx.lock();
		sceneBleList[sceneBle->GetId()] = sceneBle;
		sceneBleListMtx.unlock();
	}
	return sceneBle;
}

Room *Gateway::AddNewRoom(Room *room, bool addDatabase)
{
	if (room)
	{
		roomList[room->GetId()] = room;
		if (addDatabase)
		{
			database->RoomAdd(room);
		}
	}
	AddNewGroup(room, false);
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
	return data;
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

void Gateway::setAutoOta(bool isAutoOta)
{
	this->isAutoOta = isAutoOta;
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
				Util::ExecuteCMD("echo 1 > /sys/class/st_relay/relay1");
			}
			else
			{
				Util::ExecuteCMD("echo 0 > /sys/class/st_relay/relay1");
			}
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_RELAY "1") && dataValue[KEY_ATTRIBUTE_RELAY "1"].isInt())
		{
			onoff = dataValue[KEY_ATTRIBUTE_RELAY "1"].asInt();
			if (onoff)
			{
				Util::ExecuteCMD("echo 1 > /sys/class/st_relay/relay2");
			}
			else
			{
				Util::ExecuteCMD("echo 0 > /sys/class/st_relay/relay2");
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

int Gateway::pushStartAddHc(Json::Value &dataValue)
{
	return PublishToLocalMessage("startAddHc", dataValue, "startAddHcRsp", NULL, 0);
}

int Gateway::pushStopAddHc(Json::Value &dataValue)
{
	return PublishToLocalMessage("stopAddHc", dataValue, "stopAddHcRsp", NULL, 0);
}

int Gateway::pushNotify(Json::Value &dataValue)
{
	return PublishToLocalMessage("newNotify", dataValue, "newNotifyRsp", NULL, 0);
}

int Gateway::pushMsgHcCoreToHcApp(string cmd, string id, string name, Json::Value &listDevice, string roomId)
{
#ifdef __ANDROID__
	Json::Value msg;
	msg["cmd"] = cmd;
	msg["rqi"] = Util::genRandRQI(16);
	msg["data"]["id"] = id;
	msg["data"]["name"] = name;
	msg["data"]["devices"] = listDevice;
	if (roomId != "")
		msg["data"]["roomId"] = roomId;
	return PublishToLocalMessage(msg);
#else
	return CODE_OK;
#endif
}

string Gateway::CreateJsonGroupSceneSendHcCoreToHcApp(string cmd, string id, string name, Json::Value &listDevice, string roomId)
{
	Json::Value msg;
	msg["cmd"] = cmd;
	msg["rqi"] = Util::genRandRQI(16);
	msg["data"]["id"] = id;
	msg["data"]["name"] = name;
	msg["data"]["devices"] = listDevice;
	if (roomId != "")
		msg["data"]["roomId"] = roomId;
	return msg.toString();
}

void Gateway::printGroup()
{
	for (auto &[id, grp] : groupList)
	{
		LOGI("group: %s", id.c_str());
		for (auto &dev : grp->deviceList)
		{
			LOGI("\tdev:%s: %d", dev->device->GetId().c_str(), dev->epId);
		}
	}
}

time_t Gateway::getLastTimePingGwBle()
{
	return this->lastTimePingGwBle;
}

void Gateway::setLastTimePingGwBle(time_t timeUpdate)
{
	this->lastTimePingGwBle = timeUpdate;
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
			LOGI("\tdev:%s: %d: %d", dev->device->GetId().c_str(), dev->device->GetAddr(), dev->epId);
		}
	}
}

#ifdef __ANDROID__
int Gateway::CreateNoti(Noti *noti, bool addDatabase, bool pushNoti)
{
	if (addDatabase)
	{
		database->NotiAdd(noti);
	}
	if (pushNoti)
	{
		Json::Value notiValue;
		notiValue["cmd"] = "newNotify";
		notiValue["rqi"] = Util::genRandRQI(16);
		notiValue["data"] = Json::arrayValue;
		Json::Value payloadJson;
		payloadJson.parse(noti->GetContent());
		payloadJson["isRead"] = false;
		notiValue["data"].append(payloadJson);
		PublishToLocalMessage(notiValue);
	}
	notiList[noti->GetId()] = noti;
	return CODE_OK;
}

int Gateway::DelNoti(Noti *noti)
{
	database->NotiDel(noti);
	notiList.erase(noti->GetId());
	return CODE_OK;
}

Noti *Gateway::getNotifromId(string id)
{
	if (notiList.find(id) != notiList.end())
	{
		return notiList[id];
	}
	return NULL;
}

Json::Value Gateway::BuildJsonDataNoti(Device *device, string id, string type, string content)
{
	Json::Value notiData;
	notiData["id"] = id;
	notiData["type"] = type;
	notiData["content"] = content;
	notiData["title"] = device->GetName();
	notiData["time"] = to_string(time(NULL));
	notiData["location"] = "";
	for (auto temp : roomList)
	{
		Room *room = temp.second;
		if (room->GetPositionDevice(device, device->GetAddr()) != CODE_ERROR)
		{
			notiData["location"] = room->GetName();
		}
	}
	return notiData;
}
#endif
