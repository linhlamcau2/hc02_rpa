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
#include "RelayProtocol.h"
#include "Product.h"

Gateway *gateway = NULL;
static bool check_connect_cloud = false;
static uint32_t tick_count_qr_scan = 0;
Gateway::Gateway(string mac, string address, int port, string clientId, string username, string password, int keepalive,
				 string localAddress, int localPort, string localUsername, string localPassword, int localKeepalive)
	: CloudProtocol(mac, address, port, clientId, username, password, keepalive, false),
	  LocalProtocol(mac, localAddress, localPort, mac, localUsername, localPassword, localKeepalive, false)
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
		check_connect_cloud = true;
#ifdef ESP_PLATFORM
		tick_count_qr_scan = xTaskGetTickCount();
#endif
		Util::LedInternet(true);
		gpioProtocol->set_led_warning(0);
#ifdef ESP_PLATFORM
		SetLedInternet(true);
#endif
	}
	else
	{
		check_connect_cloud = false;
		Util::LedInternet(false);
		gpioProtocol->set_led_warning(1);
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
	// bleProtocol->ResetBle();
	Util::ExecuteCMD("echo '0' > /sys/class/gpio/gpio1/value");
	sleep(1);
	Util::ExecuteCMD("echo '1' > /sys/class/gpio/gpio1/value");
#elif defined(ESP_PLATFORM)
	SetGpioResetGwBle();
#endif
	return CODE_OK;
}

uint32_t timeout;
bool checkRssi = false;
bool checkRelayOn[4] = {false};
bool checkRelayOff[4] = {false};
bool checkOnAll = false;
bool checkOffAll = false;
bool check_proc_success = true;
bool check_pair_k9b = false;
bool check_stt_last = false;
bool check_all_process = false;
bool begin = false;

uint32_t deviceType = 0;
uint16_t deviceVersion = 0;
bool check_res_on[4] = {false};
bool check_res_off[4] = {false};

uint8_t process_test_ctcu(uint8_t num_ele, int pos)
{
	uint8_t err = 1;

	uint8_t dev_mac[6] = {0};
	Util::ConvertStringToHex(qrProtocol->mac, dev_mac, 6);
	bleProtocol->GetDeviceType(dev_mac, qrProtocol->addr, deviceType, deviceVersion);

	bleProtocol->Request_Training(0, qrProtocol->addr); // Buoc 2: dung test luyen, chuan bi test tinh nang
	SLEEP_MS(1000);
	bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, 255, 0);
	SLEEP_MS(1500);

	for (int i = 0; i < num_ele; i++) // Buoc 3: diueu khien chu trinh 2 lan
	{
		if (bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, i + 1 - pos, 1) == CODE_OK)
		{
			check_res_on[i] = true;
		}
	}

	SLEEP_MS(1500);
	for (int i = 0; i < num_ele; i++)
	{
		checkRelayOn[i] = (check_res_on[i] && !gpioProtocol->gpio_get(i)) ? true : false;
		if (!checkRelayOn[i])
			err = 0;
	}

	for (int i = 0; i < num_ele; i++)
	{
		if (bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, i + 1 -pos, 0) == CODE_OK)
		{
			check_res_off[i] = true;
		}
	}

	SLEEP_MS(500);
	for (int i = 0; i < num_ele; i++)
	{
		checkRelayOff[i] = (!(!gpioProtocol->gpio_get(i)) && check_res_off[i]) ? true : false;
		if (!checkRelayOff[i])
			err = 0;
	}

	bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, 255, 1);
	SLEEP_MS(500);

	checkOnAll = true;
	for (int i = 0; i < num_ele; i++)
	{
		if (!gpioProtocol->gpio_get(i) == 0)
		{
			checkOnAll = false;
			err = 0;
			;
			break;
		}
	}

	bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, 255, 0);
	SLEEP_MS(500);

	checkOffAll = true;
	for (int i = 0; i < num_ele; i++)
	{
		if (!gpioProtocol->gpio_get(i) == 1)
		{
			checkOffAll = false;
			err = 0;
			;
			break;
		}
	}

	if (bleProtocol->Request_Pair_K9B(qrProtocol->addr, 0xff, qrProtocol->mac_k9b_int, 2) == CODE_OK)
	{
		SLEEP_MS(1500);
		gpioProtocol->gpio_supply_power_k9b();
		SLEEP_MS(1000);
		check_pair_k9b = true;
	}
	else
		return 0;

	check_stt_last = true;
	for (int i = 0; i < num_ele; i++)
	{
		if (!gpioProtocol->gpio_get(i) == 0)
		{
			check_stt_last = false;
			err = 0;
			break;
		}
	}

	return err;
}

void rd_reporting_proc_ctcu(uint8_t num_ele, uint8_t err, string dev_type)
{
	Json::Value rs;
	// rs["mac"] = qrProtocol->mac;
	// rs["addr"] = qrProtocol->addr;
	rs["version"] = to_string(deviceVersion);
	rs["deviceType"] = qrProtocol->prod_num + qrProtocol->prod_code + qrProtocol->serial;
	rs["serial"] = dev_type;
	rs["rssi"] = checkRssi ? bleProtocol->rssi : 0;

	rs["on_relay1"] = checkRelayOn[0];
	rs["off_relay1"] = checkRelayOff[0];
	if (num_ele > 1)
	{
		rs["on_relay2"] = checkRelayOn[1];
		rs["off_relay2"] = checkRelayOff[1];
	}
	if (num_ele > 2)
	{
		rs["on_relay3"] = checkRelayOn[2];
		rs["off_relay3"] = checkRelayOff[2];
	}
	if (num_ele > 3)
	{
		rs["on_relay4"] = checkRelayOn[3];
		rs["off_relay4"] = checkRelayOff[3];
	}

	rs["on_all"] = checkOnAll;
	rs["off_all"] = checkOffAll;
	rs["remote_learn"] = check_pair_k9b;
	rs["remote_control"] = check_stt_last;

	Json::Value deviceJson = Json::arrayValue;
	deviceJson.append(rs);
	Json::Value dataPush;
	Json::Value devJson;

	devJson["device"] = deviceJson;
	dataPush["cmd"] = "hcReportLog";
	dataPush["rqi"] = Util::genRandRQI(16);
	dataPush["data"] = devJson;

	LOGE("%s", dataPush.toString().c_str());
	gateway->CloudPublish(dataPush.toString());
	if (!err)
	{
		gpioProtocol->set_led_fail();
	}
	else
	{
		gpioProtocol->set_led_success();
	}
	gateway->RestartBleGw();

	SLEEP_MS(10000);
}

void start_process()
{
	gpioProtocol->reset_led_in_proc();
	checkRssi = false;
	checkOnAll = false;
	checkOffAll = false;

	check_proc_success = true;
	check_pair_k9b = false;
	check_stt_last = false;

	deviceType = 0;
	deviceVersion = 0;
	for (int i = 0; i < 4; i++)
	{
		checkRelayOn[i] = false;
		checkRelayOff[i] = false;
		check_res_on[i] = false;
		check_res_on[i] = false;
	}
}

int Gateway::TestSwitch()
{

	while (1)
	{
		if (check_connect_cloud && qrProtocol->startTest == 0)
		{
			// if (xTaskGetTickCount() - tick_count_qr_scan > 1000 * 10 / portTICK_PERIOD_MS)
			// {
			// 	string a = "QR_SCAN FAILED";
			// 	this->CloudPublish(a);
			// 	tick_count_qr_scan = xTaskGetTickCount();
			// }
		}
		if (qrProtocol->startTest && check_connect_cloud)
		{
			start_process();
			ProductInfo prod;
			if (is_product_exist(qrProtocol->prod_code, prod))
			{
				switch (prod.type)
				{
				case CTCU_BLE_CN_O4T:
				case CTCU_BLE_CN_O3T:
				case CTCU_BLE_CN_O2T:
				case CTCU_BLE_CN_O1T:
				{
					uint8_t num_ele = prod.num_ele;
					uint8_t err = process_test_ctcu(num_ele, 0);
					rd_reporting_proc_ctcu(num_ele, err, prod.dev_type);
					break;
				}
				case CTCU_WF_CN_01T_2W_SP:
				case CTCU_WF_CN_02T_2W_SP:
				case CTCU_WF_CN_03T_2W_SP:
				case CTCU_WF_CN_04T_2W_SP:
				{
					uint8_t num_ele = prod.num_ele;
					uint8_t err = process_test_ctcu(num_ele,1);
					rd_reporting_proc_ctcu(num_ele, err, prod.dev_type);
					break;
				}
				default:
					break;
				}
			}

#ifdef ESP_PLATFORM
			tick_count_qr_scan = xTaskGetTickCount();
#endif
			qrProtocol->startTest = false;
		}
		SLEEP_MS(1000);
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