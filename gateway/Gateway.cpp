#include "Gateway.h"
#include "Log.h"
#include <unistd.h>
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include "json.h"
#include "Util.h"
#include "Wifi.h"
#include "Base64.h"
#include "Config.h"
#include <poll.h>

#ifdef ESP_PLATFORM
#include "Config.h"
#include "Led.h"
#include "esp_spiffs.h"
#endif

#include "QrProtocol.h"
#include "gpioProtocol.h"
#include "RelayProtocol.h"
#include "UartDebugProtocol.h"
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
	// LocalProtocol::init();
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
	// LocalConnect();
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



enum
{
	RELAY_LIGHTING = 0,
	RELAY_EXHAUST_FAN,
	RELAY_COOL_FAN,
	RELAY_HEATING_LOW,
	RELAY_HEATING_HIGH,
	RELAY_MAX_ID
};

enum 
{
	PCBA_TEST_TC =0,
	PCBA_TEST_SMT ,
};

static bool id_relay_check[RELAY_MAX_ID] = {0};
string rqi;
string serial;
string version;
uint8_t type_test_pcba_dhpt = PCBA_TEST_TC;
bool is_ready_test_pcba = false;

void active_test_pcba_dhpt(const string& rqi_recv,const string& serial_recv,const string& ver_recv,const string& cmd)
{
	if(!is_ready_test_pcba)
	{
		if (cmd == "startTestPCBASmt") type_test_pcba_dhpt = PCBA_TEST_SMT;
		else type_test_pcba_dhpt = PCBA_TEST_TC;
		rqi = rqi_recv;
		serial = serial_recv;
		version = ver_recv;
		is_ready_test_pcba = true;
	}
}
auto get_sub_string = [](const std::string &s){
	size_t pos = s.find('-');
	return (pos != std::string::npos)? s.substr(pos +1) : std::string{};
};

void process_test_dhpt()
{
	string mac = get_sub_string(serial);
	uint16_t addr = getLast4HexAsUint16(mac);
	addr = (addr > 0x8000 ) ? (addr - 0x8000) : addr;
	memset(id_relay_check,1,sizeof(id_relay_check));
	for(int i =0; i< RELAY_MAX_ID; ++i)
	{
		bleProtocol -> Ctrl_Relay_DHPT(addr,i+1,1);
	}

	SLEEP_MS(500);
	for(int i =0; i< RELAY_MAX_ID; ++i)
	{
		if(!gpioProtocol -> gpio_get_pin_test_dhpt(i,type_test_pcba_dhpt)) id_relay_check[i] = 0;
	}

	for(int i = RELAY_MAX_ID-1; i>=0; --i)
	{
		bleProtocol -> Ctrl_Relay_DHPT(addr,i+1,0);
	}

	SLEEP_MS(500);
	for(int i =0; i< RELAY_MAX_ID; ++i)
	{
		if(gpioProtocol -> gpio_get_pin_test_dhpt(i,type_test_pcba_dhpt)) id_relay_check[i] = 0;
	}
}

void report_to_server()
{
Json::Value rs;
	// rs["mac"] = qrProtocol->mac;
	// rs["addr"] = qrProtocol->addr;
	rs["version"] = version;
	rs["serial"] = serial;
	rs["deviceType"] = "none";
	rs["rlCoolingFanPcbaTc"] = (id_relay_check[RELAY_COOL_FAN]) ;
	rs["rlExhaustFanPcbaTc"] = (id_relay_check[RELAY_EXHAUST_FAN]) ;
	rs["rlLightPcbaTc"] = (id_relay_check[RELAY_LIGHTING]) ;
	rs["rlHeatLowPcbaTc"] = (id_relay_check[RELAY_HEATING_LOW]) ;
	rs["rlHeatHighPcbaTc"] = (id_relay_check[RELAY_HEATING_HIGH]) ;

	Json::Value deviceJson = Json::arrayValue;
	deviceJson.append(rs);
	Json::Value dataPush;
	Json::Value devJson;

	devJson["device"] = deviceJson;
	dataPush["cmd"] = "hcReportLogPcbaTcAirConditionLight";
	// dataPush["rqi"] = Util::genRandRQI(16);
	dataPush["rqi"] = rqi;
	dataPush["data"] = devJson;

	LOGE("%s", dataPush.toString().c_str());
	gateway->CloudPublish(dataPush.toString());
}

int Gateway::TestSwitch()
{
	while (1)
	{
		if(is_ready_test_pcba)
		{
			process_test_dhpt();
			report_to_server();
			is_ready_test_pcba = false;	
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