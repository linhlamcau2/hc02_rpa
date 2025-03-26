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
#include "Util.h"
#include "Wifi.h"
#include "Base64.h"
#include "Config.h"

#ifdef ESP_PLATFORM
#include "Config.h"
#include "Led.h"
#include "esp_spiffs.h"
#endif

#include "QrProtocol.h"
#include "RelayProtocol.h"

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
}

Gateway::~Gateway()
{
}

#ifdef ESP_PLATFORM

static void TestSwitchThread(void *data)
{
	Gateway *gateway = (Gateway *)data;
	gateway->TestSwitch();
	vTaskDelete(NULL);
}
#endif

void Gateway::init()
{
	// Device::InitDeviceModelList();
	LocalProtocol::init();
	CloudProtocol::init();
	Udp::init();

	InitUdpMessage();
	// InitMqttMessageDevice();
	// InitMqttMessageGroup();
	// InitMqttMessageRoom();
	// InitMqttMessageRule();
	// InitMqttMessageScene();
	// InitMqttMessageHc();

#ifdef ESP_PLATFORM
	LOGI("Free memory: %d bytes, internal: %d bytes", esp_get_free_heap_size(), esp_get_free_internal_heap_size());
	if (xTaskCreate(TestSwitchThread, "CheckOnline", 5120, this, 7, NULL) != pdPASS)
	{
		LOGE("Failed to create task");
		SetLedService(false);
	}
	vTaskDelay(10);
#else
	thread testSwitchThread(bind(&Gateway::TestSwitch, this));
	testSwitchThread.detach();
#endif
	LocalConnect();
	CloudConnect();
}

void Gateway::OnCloudConnect(bool isConnected, bool isReconnect)
{
	LOGI("OnCloudConnect: %d", isConnected);

	if (isConnected)
	{
		Util::LedInternet(true);
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
			// bleProtocol->SetGwAddr(qrProtocol->addr, 0);
			if ((bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 1, 1, 1) == CODE_OK))
			{
				usleep(600000);
				if (relayProtocol->rl1 == 1)
					checkRelay1On = true;
			}
			if ((bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 1, 2, 1) == CODE_OK))
			{
				usleep(600000);
				if (relayProtocol->rl2 == 1)
					checkRelay2On = true;
			}
			if ((bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 1, 3, 1) == CODE_OK))
			{
				usleep(600000);
				if (relayProtocol->rl3 == 1)
					checkRelay3On = true;
			}
			if ((bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 1, 4, 1) == CODE_OK))
			{
				usleep(600000);
				if (relayProtocol->rl4 == 1)
					checkRelay4On = true;
			}

			if ((bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 1, 1, 0) == CODE_OK))
			{
				usleep(600000);
				if (relayProtocol->rl1 == 0)
					checkRelay1Off = true;
			}
			if ((bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 1, 2, 0) == CODE_OK))
			{
				usleep(600000);
				if (relayProtocol->rl2 == 0)
					checkRelay2Off = true;
			}
			if ((bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 1, 3, 0) == CODE_OK))
			{
				usleep(600000);
				if (relayProtocol->rl3 == 0)
					checkRelay3Off = true;
			}
			if ((bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 1, 4, 0) == CODE_OK))
			{
				usleep(600000);
				if (relayProtocol->rl4 == 0)
					checkRelay4Off = true;
			}

			bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 1, 255, 1);
			usleep(1500000);
			if (relayProtocol->rl1 && relayProtocol->rl2 && relayProtocol->rl3 && relayProtocol->rl4)
				checkOnAll = true;
			bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 1, 255, 0);
			usleep(1500000);
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