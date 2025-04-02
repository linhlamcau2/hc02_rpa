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
#include "gpioProtocol.h"
// #include "RelayProtocol.h"

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
	bool checkRelayOn[4] = {false};
	bool checkRelayOff[4] = {false};
	bool checkOnAll;
	bool checkOffAll;
	bool check_proc_success = true;
	while (1)
	{
		if (qrProtocol->startTest)
		{
			gpioProtocol->reset_led_in_proc();
			checkRssi = false;
			checkOnAll = false;
			checkOffAll = false;

			bleProtocol->StartScan(); // Buoc 1: bat dau quet
			bleProtocol->isMatchMac = false;
			timeout = 3000;
			while (!bleProtocol->isMatchMac && timeout--)
			{
				SLEEP_MS(1);
			}
			bleProtocol->StopScan();
			if (bleProtocol->isMatchMac)
			{
				checkRssi = true;
			}
			else
			{
				goto noti_fail;
			}
			bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, 255, 0);
			// bleProtocol->SetGwAddr(qrProtocol->addr, 0);
			SLEEP_MS(1000);
			bleProtocol->Request_Training(0, qrProtocol->addr); // Buoc 2: dung test luyen, chuan bi test tinh nang
			SLEEP_MS(1000);
			bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, 255, 0);
			SLEEP_MS(1000);
			for (int i = 0; i < 4; i++) // Buoc 3: diueu khien chu trinh 2 lan
			{
				if (bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, i + 1, 1) == CODE_OK)
				{
					SLEEP_MS(1000);
					checkRelayOn[i] = (gpioProtocol->gpio_get(i)) ? true : false;
					if (!checkRelayOn[i])
						check_proc_success = false;
				}
			}
			for (int i = 0; i < 4; i++)
			{
				if (bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, i + 1, 0) == CODE_OK)
				{
					SLEEP_MS(1000);
					checkRelayOff[i] = (!gpioProtocol->gpio_get(i)) ? true : false;
					if (!checkRelayOff[i])
						check_proc_success = false;
				}
			}
			LOGE("tp1");
			bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, 255, 1);
			SLEEP_MS(3000);
			LOGE("tp2");
			checkOnAll = ((gpioProtocol->gpio_get(0) && gpioProtocol->gpio_get(1) && gpioProtocol->gpio_get(2) && gpioProtocol->gpio_get(3))) ? true : false;
			bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, 255, 0);
			SLEEP_MS(3000);
			LOGE("tp3");
			checkOffAll = (!gpioProtocol->gpio_get(0) && !gpioProtocol->gpio_get(1) && !gpioProtocol->gpio_get(2) && !gpioProtocol->gpio_get(3)) ? true : false;
			LOGE("tp4");

			Json::Value rs;
			rs["mac"] = qrProtocol->mac;
			rs["addr"] = qrProtocol->addr;
			rs["rssi"] = checkRssi ? bleProtocol->rssi : 0;
			rs["on_relay1"] = checkRelayOn[0];
			rs["on_relay2"] = checkRelayOn[1];
			rs["on_relay3"] = checkRelayOn[2];
			rs["on_relay4"] = checkRelayOn[3];
			rs["off_relay1"] = checkRelayOff[0];
			rs["off_relay2"] = checkRelayOff[1];
			rs["off_relay3"] = checkRelayOff[2];
			rs["off_relay4"] = checkRelayOff[3];
			rs["on_all"] = checkOnAll;
			rs["off_all"] = checkOffAll;
			Json::Value deviceJson = Json::arrayValue;
			deviceJson.append(rs);
			LOGE("tp5");

			Json::Value dataPush;
			dataPush["cmd"] = "hcReportLog",
			dataPush["rpi"] = Util::genRandRQI(16),
			dataPush["device"] = deviceJson;
			LOGE("%s", dataPush.toString().c_str());
			this->CloudPublish(dataPush.toString());

			if (!(check_proc_success && checkOnAll && checkOffAll))
			{
				goto noti_fail;
			}
			LOGE("tp6");


			// if (qrProtocol->isMac_k9b)
			if(1)
			{
				bleProtocol->Request_Pair_K9B(qrProtocol->addr, 0xff, qrProtocol->mac_k9b_int, 2);
				SLEEP_MS(2000);

				gpioProtocol->gpio_supply_power_k9b();
				// SLEEP_MS(500);
				// gpioProtocol->gpio_supply_power_k9b();

				SLEEP_MS(1000);

				gpioProtocol->gpio_supply_power_k9b();
				SLEEP_MS(1000);

				gpioProtocol->gpio_supply_power_k9b();
				// SLEEP_MS(3000);

				// gpioProtocol->gpio_supply_power_k9b();
				SLEEP_MS(1000);

			}
			LOGE("tp7");

			gpioProtocol->set_led_success();
			SetGpioResetGwBle();
			qrProtocol->startTest = false;
		}
	noti_fail:
		if (qrProtocol->startTest)
		{
			LOGI("process fail");
			qrProtocol->startTest = false;
			gpioProtocol->set_led_fail();
			check_proc_success = true;
			LOGE("tp8");
		}
		SLEEP_MS(2000);
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