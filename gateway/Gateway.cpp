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
	// thread testSwitchThread(bind(&Gateway::TestSwitch, this));
	// testSwitchThread.detach();

	//Test_PCBA_DHPT_SMT_POS0 Test_PCBA_DHPT_TC() int Test_PCBA_DHPT_SMT(); int Gateway::Test_PCBA_DHPT_TC()
	thread test_dhpt_smt_thread(bind(&Gateway::Test_PCBA_DHPT_SMT, this));
	test_dhpt_smt_thread.detach();

	thread test_dhpt_tc_thread(bind(&Gateway::Test_PCBA_DHPT_TC, this));
	test_dhpt_tc_thread.detach();

	// thread test_dhpt_smt_pos0_thread(bind(&Gateway::Test_PCBA_DHPT_SMT_POS0, this));
	// test_dhpt_smt_pos0_thread.detach();

	// thread test_dhpt_smt_pos1_thread(bind(&Gateway::Test_PCBA_DHPT_SMT_POS1, this));
	// test_dhpt_smt_pos1_thread.detach();

	// thread test_dhpt_tc_pos0_thread(bind(&Gateway::Test_PCBA_DHPT_TC_POS0, this));
	// test_dhpt_tc_pos0_thread.detach();

	// thread test_dhpt_tc_pos1_thread(bind(&Gateway::Test_PCBA_DHPT_TC_POS1, this));
	// test_dhpt_tc_pos1_thread.detach();

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

typedef struct inf_test_dhpt_s
{
	int type_test;
	string rqi;
	string serial;
	string version;
	bool is_ready_test;
	bool stt_relay_check[RELAY_MAX_ID];
} inf_test_dhpt_t;

typedef struct inf_test_smt_t
{
	string rqi;
	string serial;
	string version;
}inf_test_smt_t;

inf_test_smt_t inf_test_smt[2];
inf_test_dhpt_t inf_test_dhpt[PCBA_TEST_DHPT_MAX_ID];
bool is_ready_test_pcba_smt = false;
bool is_ready_test_pcba_tc = false;

void active_test_pcba_dhpt_smt()
{
	if(is_ready_test_pcba_smt) return;
	is_ready_test_pcba_smt = true;
}

void active_test_pcba_dhpt_tc()
{
	if(is_ready_test_pcba_tc) return;
	is_ready_test_pcba_tc = true;
}

int type_test_check_dhpt(const string& cmd, int pos)
{
	if (cmd == "startTestPCBASmt")
		return PCBA_TEST_SMT_POS0 + pos;
	else if (cmd == "startTestPCBATc")
		return PCBA_TEST_TC_POS0 + pos;
	else 
		return -1;
}

void active_test_pcba_dhpt(const string& rqi_recv,const string& serial_recv,const string& ver_recv,const string& cmd, int pos)
{
	int type_test = type_test_check_dhpt(cmd,pos);
	if(type_test == -1) return;
	if(!inf_test_dhpt[type_test].is_ready_test)
	{
		inf_test_dhpt[type_test].rqi = rqi_recv;
		inf_test_dhpt[type_test].serial = serial_recv;
		inf_test_dhpt[type_test].version = ver_recv;
		memset(inf_test_dhpt[type_test].stt_relay_check,1,sizeof(inf_test_dhpt[type_test].stt_relay_check));
		inf_test_dhpt[type_test].is_ready_test = true;
	}
}

auto get_sub_string = [](const std::string &s){
	size_t pos = s.find('-');
	return (pos != std::string::npos)? s.substr(pos +1) : std::string{};
};

void process_test_pcba_dhpt(int type_test)
{
	string mac = get_sub_string(inf_test_dhpt[type_test].serial);
	
	uint16_t addr = getLast4HexAsUint16(mac);
	addr = (addr > 0x8000 ) ? (addr - 0x8000) : addr;
	cout << "addr hex" << addr << endl;
	for(int i =0; i< RELAY_MAX_ID; ++i)
	{
		bleProtocol -> Ctrl_Relay_DHPT(addr,i+1,1);
		SLEEP_MS(500);
	}

	SLEEP_MS(500);
	for(int i =0; i< RELAY_MAX_ID; ++i)
	{
		if(!gpioProtocol -> gpio_get_pin_test_dhpt(i,type_test)) inf_test_dhpt[type_test].stt_relay_check[i] = 0;
	}

	for(int i = RELAY_MAX_ID-1; i>=0; --i)
	{
		bleProtocol -> Ctrl_Relay_DHPT(addr,i+1,0);
		SLEEP_MS(500);
	}

	SLEEP_MS(500);
	for(int i = RELAY_MAX_ID-1; i>=0; --i)
	{
		if(gpioProtocol -> gpio_get_pin_test_dhpt(i,type_test)) inf_test_dhpt[type_test].stt_relay_check[i] = 0;
	}
}

void process_test_pcba_tc_dhpt(int type_test)
{
	string mac = get_sub_string(inf_test_dhpt[type_test].serial);
	
	uint16_t addr = getLast4HexAsUint16(mac);
	addr = (addr > 0x8000 ) ? (addr - 0x8000) : addr;
	cout << "addr hex" << addr << endl;
	bleProtocol -> Ctrl_Relay_DHPT(addr,0xff,1);
	SLEEP_MS(1000);
	for(int i =0; i< RELAY_MAX_ID; ++i)
	{
		if(!gpioProtocol -> gpio_get_pin_test_dhpt(i,type_test)) inf_test_dhpt[type_test].stt_relay_check[i] = 0;
	}
	bleProtocol -> Ctrl_Relay_DHPT(addr,0xff,0);
	SLEEP_MS(1000);
	for(int i = RELAY_MAX_ID-1; i>=0; --i)
	{
		if(gpioProtocol -> gpio_get_pin_test_dhpt(i,type_test)) inf_test_dhpt[type_test].stt_relay_check[i] = 0;
	}
}

void report_to_server(int type_test)
{
Json::Value rs;
	// rs["mac"] = qrProtocol->mac;
	// rs["addr"] = qrProtocol->addr;
	rs["version"] = inf_test_dhpt[type_test].version;
	rs["serial"] = inf_test_dhpt[type_test].serial;
	rs["deviceType"] = "none";
	if(type_test == PCBA_TEST_TC_POS0 || type_test == PCBA_TEST_TC_POS1)
	{
		rs["rlCoolingFanPcbaTc"] = (inf_test_dhpt[type_test].stt_relay_check[RELAY_COOL_FAN]) ;
		rs["rlExhaustFanPcbaTc"] = (inf_test_dhpt[type_test].stt_relay_check[RELAY_EXHAUST_FAN]) ;
		rs["rlLightPcbaTc"] = (inf_test_dhpt[type_test].stt_relay_check[RELAY_LIGHTING]) ;
		rs["rlHeatLowPcbaTc"] = (inf_test_dhpt[type_test].stt_relay_check[RELAY_HEATING_LOW]) ;
		rs["rlHeatHighPcbaTc"] = (inf_test_dhpt[type_test].stt_relay_check[RELAY_HEATING_HIGH]) ;
	}
	else if(type_test == PCBA_TEST_SMT_POS0 || type_test == PCBA_TEST_SMT_POS1)
	{
		rs["rlCoolingFanPcbaSmt"] = (inf_test_dhpt[type_test].stt_relay_check[RELAY_COOL_FAN]) ;
		rs["rlExhaustFanPcbaSmt"] = (inf_test_dhpt[type_test].stt_relay_check[RELAY_EXHAUST_FAN]) ;
		rs["rlLightPcbaSmt"] = (inf_test_dhpt[type_test].stt_relay_check[RELAY_LIGHTING]) ;
		rs["rlHeatLowPcbaSmt"] = (inf_test_dhpt[type_test].stt_relay_check[RELAY_HEATING_LOW]) ;
		rs["rlHeatHighPcbaSmt"] = (inf_test_dhpt[type_test].stt_relay_check[RELAY_HEATING_HIGH]) ;
	}

	Json::Value deviceJson = Json::arrayValue;
	deviceJson.append(rs);
	Json::Value dataPush;
	Json::Value devJson;

	devJson["device"] = deviceJson;
	dataPush["cmd"] = (type_test == PCBA_TEST_TC_POS0 || type_test == PCBA_TEST_TC_POS1) ?"hcReportLogPcbaTcAirConditionLight" : "hcReportLogPcbaSmtAirConditionLight";
	// dataPush["rqi"] = Util::genRandRQI(16);
	dataPush["rqi"] = inf_test_dhpt[type_test].rqi;
	dataPush["data"] = devJson;

	LOGE("%s", dataPush.toString().c_str());
	gateway->CloudPublish(dataPush.toString());
}
int Gateway::Test_PCBA_DHPT_SMT()
{
	while(1)
	{
		if(is_ready_test_pcba_smt)
		{
			for(int i=0; i<2; ++i)
			{
				if(inf_test_dhpt[PCBA_TEST_SMT_POS0+i].is_ready_test)
				{
					cout<<"start Test PCBA SMT POS"<<i<<endl;
					start_process_test_smt(i);
					// restart_chip_tlsr8253(0);
					process_test_pcba_dhpt(PCBA_TEST_SMT_POS0+i);
					report_to_server(PCBA_TEST_SMT_POS0+i);
					end_process_test_smt(i);
					inf_test_dhpt[PCBA_TEST_SMT_POS0+i].is_ready_test = false;
				}
			}
			is_ready_test_pcba_smt = false;	
		}
		SLEEP_MS(1000);
	}
}

int Gateway::Test_PCBA_DHPT_TC()
{
	while(1)
	{
		if(is_ready_test_pcba_tc)
		{
			for(int i=0; i<2; ++i)
			{
				if(inf_test_dhpt[PCBA_TEST_TC_POS0+i].is_ready_test)
				{
					cout<<"start Test PCBA TC POS"<<i<<endl;
					process_test_pcba_tc_dhpt(PCBA_TEST_TC_POS0+i);
					report_to_server(PCBA_TEST_TC_POS0+i);
					inf_test_dhpt[PCBA_TEST_TC_POS0+i].is_ready_test = false;
				}
			}
			is_ready_test_pcba_tc = false;	
		}
		SLEEP_MS(1000);
	}
}

int Gateway::Test_PCBA_DHPT_SMT_POS0()
{
	while(1)
	{
		if(inf_test_dhpt[PCBA_TEST_SMT_POS0].is_ready_test && is_ready_test_pcba_smt)
		{
			cout<<"start Test PCBA SMT POS0"<<endl;
			start_process_test_smt(0);
			// restart_chip_tlsr8253(0);
			process_test_pcba_dhpt(PCBA_TEST_SMT_POS0);
			report_to_server(PCBA_TEST_SMT_POS0);
			end_process_test_smt(0);
			inf_test_dhpt[PCBA_TEST_SMT_POS0].is_ready_test = false;
			is_ready_test_pcba_smt = false;	
		}
		SLEEP_MS(1000);
	}
	return 1;
}

int Gateway::Test_PCBA_DHPT_SMT_POS1()
{
	while(1)
	{
		if(inf_test_dhpt[PCBA_TEST_SMT_POS1].is_ready_test && is_ready_test_pcba_smt)
		{
			cout<<"start Test PCBA SMT POS1"<<endl;
			start_process_test_smt(1);
			// restart_chip_tlsr8253(1);
			process_test_pcba_dhpt(PCBA_TEST_SMT_POS1);
			report_to_server(PCBA_TEST_SMT_POS1);
			end_process_test_smt(1);
			inf_test_dhpt[PCBA_TEST_SMT_POS1].is_ready_test = false;	
			is_ready_test_pcba_smt = false;
		}
		SLEEP_MS(1000);
	}
	return 1;
}

int Gateway::Test_PCBA_DHPT_TC_POS0()
{
	while(1)
	{
		if(inf_test_dhpt[PCBA_TEST_TC_POS0].is_ready_test)
		{
			cout<<"start Test PCBA TC POS0"<<endl;
			restart_chip_tlsr8253(0);
			process_test_pcba_dhpt(PCBA_TEST_TC_POS0);
			report_to_server(PCBA_TEST_TC_POS0);
			inf_test_dhpt[PCBA_TEST_TC_POS0].is_ready_test = false;	
		}
		SLEEP_MS(1000);
	}
	return 1;
}

int Gateway::Test_PCBA_DHPT_TC_POS1()
{
	while(1)
	{
		if(inf_test_dhpt[PCBA_TEST_TC_POS1].is_ready_test)
		{
			cout<<"start Test PCBA TC POS1"<<endl;
			restart_chip_tlsr8253(1);
			process_test_pcba_dhpt(PCBA_TEST_TC_POS1);
			report_to_server(PCBA_TEST_TC_POS1);
			inf_test_dhpt[PCBA_TEST_TC_POS1].is_ready_test = false;	
		}
		SLEEP_MS(1000);
	}
	return 1;
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