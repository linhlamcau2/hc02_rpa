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
	SLEEP_MS(500);
	bleProtocol->Request_Training(0, qrProtocol->addr); // Buoc 2: dung test luyen, chuan bi test tinh nang
	SLEEP_MS(1000);
	bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, 255, 0);
	SLEEP_MS(1500);

	for (int i = 0; i < num_ele; i++) // Buoc 3: diueu khien chu trinh 2 lan
	{
		if (bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, i + 1 - pos, 1) == CODE_OK)
		{
			check_res_on[i] = true;
			LOGD("nut on %d: %d", i + 1 - pos, gpioProtocol->gpio_get(i));
			SLEEP_MS(500);
		}
	}

	SLEEP_MS(1000);

	int j = 0;
	for (int i = 0; i < num_ele; i++)
	{
		if (num_ele == 1)
			j = 3;
		else
			j = i;
		checkRelayOn[i] = (check_res_on[i] && !gpioProtocol->gpio_get(j)) ? true : false;
		if (!checkRelayOn[i])
			err = 0;
	}

	for (int i = 0; i < num_ele; i++)
	{
		if (bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, i + 1 - pos, 0) == CODE_OK)
		{
			check_res_off[i] = true;
			LOGD("nut off %d: %d", i + 1 - pos, gpioProtocol->gpio_get(i));
			SLEEP_MS(500);
		}
	}

	SLEEP_MS(1000);
	for (int i = 0; i < num_ele; i++)
	{
		if (num_ele == 1)
			j = 3;
		else
			j = i;
		checkRelayOff[i] = (!(!gpioProtocol->gpio_get(j)) && check_res_off[i]) ? true : false;
		if (!checkRelayOff[i])
			err = 0;
	}

	bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, 255, 1);
	SLEEP_MS(1000);

	checkOnAll = true;
	for (int i = 0; i < num_ele; i++)
	{
		if (num_ele == 1)
			j = 3;
		else
			j = i;
		if (!gpioProtocol->gpio_get(j) == 0)
		{
			checkOnAll = false;
			err = 0;
			break;
		}
	}

	bleProtocol->ControlRelayOfSwitch(qrProtocol->addr, 4, 255, 0);
	SLEEP_MS(1000);

	checkOffAll = true;
	for (int i = 0; i < num_ele; i++)
	{
		if (num_ele == 1)
			j = 3;
		else
			j = i;
		if (!gpioProtocol->gpio_get(j) == 1)
		{
			checkOffAll = false;
			err = 0;
			break;
		}
	}

	if (bleProtocol->Request_Pair_K9B(qrProtocol->addr, 0xff, qrProtocol->mac_k9b_int, 1) == CODE_OK)
	{
		LOGD("resp req succ k9b");
		SLEEP_MS(1500);
		gpioProtocol->gpio_supply_power_k9b();
		SLEEP_MS(1000);
		check_pair_k9b = true;
	}
	else
	{
		if (pos)
		{
			bleProtocol->resetWifiCTCU(qrProtocol->addr);
			SLEEP_MS(6000);
		}
		return 0;
	}

	check_stt_last = true;
	for (int i = 0; i < num_ele; i++)
	{
		if (num_ele == 1)
			j = 3;
		else
			j = i;
		if (!gpioProtocol->gpio_get(j) == 0)
		{
			check_stt_last = false;
			err = 0;
			break;
		}
	}

	if (pos)
	{
		bleProtocol->resetWifiCTCU(qrProtocol->addr);
		SLEEP_MS(6000);
	}
	return err;
}

void rd_reporting_proc_ctcu(uint8_t num_ele, uint8_t err, string dev_type)
{
	Json::Value rs;
	// rs["mac"] = qrProtocol->mac;
	// rs["addr"] = qrProtocol->addr;
	rs["version"] = to_string(deviceVersion);
	rs["serial"] = qrProtocol->prod_num + qrProtocol->prod_code + qrProtocol->serial + "-" + qrProtocol->mac;
	rs["deviceType"] = dev_type;
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

bool stt[3] = {false};
bool stt_k9b[3] = {false};
bool check_res = false;

void prepare_gpio(int gpio, const std::string &edge_type = "both")
{
	std::ofstream export_file("/sys/class/gpio/export");
	export_file << gpio;
	export_file.close();

	std::string base = "/sys/class/gpio/gpio" + std::to_string(gpio);
	std::ofstream dir_file(base + "/direction");
	dir_file << "in";
	dir_file.close();

	std::ofstream edge_file(base + "/edge");
	edge_file << edge_type;
	edge_file.close();
}

bool wait_for_gpio_edge(int gpio, int timeout_ms = 2000)
{
	std::string value_path = "/sys/class/gpio/gpio" + std::to_string(gpio) + "/value";
	int fd = open(value_path.c_str(), O_RDONLY | O_NONBLOCK);
	if (fd < 0)
	{
		std::cerr << "erro" << value_path << "\n";
		return false;
	}
	char buf;
	// Clear edge lần đầu
	lseek(fd, 0, SEEK_SET);
	read(fd, &buf, 1);
	usleep(10000);  // Chờ cho kernel xử lý (10ms)

	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLPRI | POLLERR;

	int ret = poll(&pfd, 1, timeout_ms);

	if (ret > 0)
	{
		lseek(fd, 0, SEEK_SET);
		read(fd, &buf, 1);
		close(fd);
		return true;
	}

	close(fd);
	return false;
}

// Hàm trả về future<bool> để chạy song song
std::future<bool> detect_pulse_async(int gpio)
{
	return std::async(std::launch::async, [gpio]()
					  { return wait_for_gpio_edge(gpio, 2000); });
}

int test_ctcc_and_ctr()
{
	uint8_t err = 1;

	uint8_t dev_mac[6] = {0};
	Util::ConvertStringToHex(qrProtocol->mac, dev_mac, 6);
	bleProtocol->GetDeviceType(dev_mac, qrProtocol->addr, deviceType, deviceVersion);
	SLEEP_MS(500);
	bleProtocol->Request_Training(0, qrProtocol->addr); // Buoc 2: dung test luyen, chuan bi test tinh nang

	bleProtocol->SetGwAddr(qrProtocol->addr, 0);

	bleProtocol->ConfigMotor(qrProtocol->addr, 1); // loai 4 day DC
	SLEEP_MS(2000);
	prepare_gpio(3);  // GPIO3: xung 1 → 0
	prepare_gpio(0);  // GPIO0: xung 1 → 0
	prepare_gpio(2);  // GPIO2: xung 0 → 1
	prepare_gpio(37); // GPIO37: xung 1 → 0

	for (int j = 0; j < 3; j++) // Buoc 3: diueu khien chu trinh 2 lan
	{
		int i = 0;
		int idx = 0;
		if (j == 0)
		{
			i = 1; // nut 1   // mo
			idx = 0;
		}
		else if (j == 1)
		{
			i = 2; // nut 2   // dung
			idx = 3; // nc
		}
		else
		{
			i = 0; // nut 3  // dong
			idx = 2;
		}
		if (1) // i: dong -> mo -> dung
		{
			auto future = detect_pulse_async(idx);
			bleProtocol->ControlOpenClosePausePercent(qrProtocol->addr, i, 0);
			stt[i] = future.get();
			if (stt[i])
				cout << "co xung" << endl;
			else
				cout << "khong co xung" << endl;
			SLEEP_MS(3000);
		}
	}

	if (bleProtocol->Request_Pair_K9B(qrProtocol->addr, 0xff, qrProtocol->mac_k9b_int, 1) == CODE_OK)
	{
		check_pair_k9b = true;
	}
	else
	{
		bleProtocol->resetWifiCTCU(qrProtocol->addr);
		return 0;
	}

	SLEEP_MS(2000);

	for (int i = 0; i < 3; i++)
	{
		int idx = 0;
		if (i == 0)
			idx = 0;
		else if (i == 1)
			idx = 3; // no
		else if (i == 2)
			idx = 2;

		auto future = detect_pulse_async(idx);
		uartDebugProtocol->SetValueButton(i);
		stt_k9b[i] = future.get();
		if (stt_k9b[i])
			cout << "co xung" << endl;
		else
			cout << "khong co xung" << endl;
		SLEEP_MS(3000);
	}
	bleProtocol->ConfigMotor(qrProtocol->addr, 3); // loai 3 day AC
	SLEEP_MS(3000);
	bleProtocol->resetWifiCTCU(qrProtocol->addr);

	return err;
}

void rd_reporting_proc_ctcc_and_ctr(uint8_t err, string dev_type)
{
	Json::Value rs;
	// rs["mac"] = qrProtocol->mac;
	// rs["addr"] = qrProtocol->addr;
	rs["version"] = to_string(deviceVersion);
	rs["serial"] = qrProtocol->prod_num + qrProtocol->prod_code + qrProtocol->serial + "-" + qrProtocol->mac;
	rs["deviceType"] = dev_type;
	// rs["rssi"] = checkRssi ? bleProtocol->rssi : 0;

	rs["open"] = stt[1];
	rs["close"] = stt[0];
	rs["stop"] = stt[2];

	rs["remote_learn"] = check_pair_k9b;
	rs["remote_control_open"] = stt_k9b[0];
	rs["remote_control_close"] = stt_k9b[2];
	rs["remote_control_stop"] = stt_k9b[1];

	err = (stt[0] && stt[1] && stt[2] && stt_k9b[0] && stt_k9b[1] && stt_k9b[2] && check_pair_k9b) ? 1 : 0;
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
	for (int i = 0; i < 3; i++)
	{
		stt[i] = false;
		stt_k9b[i] = false;
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
				case CTCU_BLE_CN_O4T_MN:
				case CTCU_BLE_CN_O3T_MN:
				case CTCU_BLE_CN_O2T_MN:
				case CTCU_BLE_CN_O1T_MN:
				case CTCU_BLE_CN_REMT:
				case CTCU_BLE_CN_REMT_MN:
				case CTCU_BLE_V_O4T:
				case CTCU_BLE_V_O3T:
				case CTCU_BLE_V_O2T:
				case CTCU_BLE_V_O1T:
				case CTCU_BLE_V_O4T_MN:
				case CTCU_BLE_V_O3T_MN:
				case CTCU_BLE_V_O2T_MN:
				case CTCU_BLE_V_O1T_MN:
				case CTCU_BLE_V_REMT:
				case CTCU_BLE_V_REMT_MN:
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
				case CTCU_WF_CN_01T_2W_SP_MN:
				case CTCU_WF_CN_02T_2W_SP_MN:
				case CTCU_WF_CN_03T_2W_SP_MN:
				case CTCU_WF_CN_04T_2W_SP_MN:
				case CTCU_WF_V_01T_2W_SP:
				case CTCU_WF_V_02T_2W_SP:
				case CTCU_WF_V_03T_2W_SP:
				case CTCU_WF_V_04T_2W_SP:
				case CTCU_WF_V_01T_2W_SP_MN:
				case CTCU_WF_V_02T_2W_SP_MN:
				case CTCU_WF_V_03T_2W_SP_MN:
				case CTCU_WF_V_04T_2W_SP_MN:
				{
					uint8_t num_ele = prod.num_ele;
					uint8_t err = process_test_ctcu(num_ele, 1);
					rd_reporting_proc_ctcu(num_ele, err, prod.dev_type);
					break;
				}

				case CTR_BLE_CN:
				case CTR_BLE_CN_MN:
				case CTR_BLE_V:
				case CTR_BLE_V_MN:
				case CTR_BLE_WF_CN:
				case CTR_BLE_WF_CN_MN:
				case CTR_BLE_WF_V:
				case CTR_BLE_WF_V_MN:
				case CTCC_BLE_CN:
				case CTCC_BLE_V:
				case CTCC_BLE_WF_CN:
				case CTCC_BLE_WF_V:
				{
					uint8_t num_ele = prod.num_ele;
					uint8_t err = test_ctcc_and_ctr();
					rd_reporting_proc_ctcc_and_ctr(err, prod.dev_type);
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