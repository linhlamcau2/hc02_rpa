#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <Log.h>
#include <json.h>
#include <signal.h>
#include "Config.h"
#include "Gateway.h"
#include "Device.h"
#include "Db.h"
#include "Util.h"
#include "Wifi.h"
#include "TimerSchedule.h"
#include "ButtonSignal.h"

#include "BleProtocol.h"
#define BLE_UART_PORT "/dev/ttyS1"

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#define ZIGBEE_UART_PORT "/dev/ttyS0"
#endif

#define TAG "MAIN"

using namespace std;

static void signal_handler(int sig)
{
	LOGI("signal_handler: %d", sig);
	if (sig == SIGUSR1)
	{
		buttonSignal->OnPress();
	}
	else if (sig == SIGUSR2)
	{
		buttonSignal->OnRelease();
	}
	signal(sig, signal_handler);
}

int main(int argc, char *argv[])
{
	log_set_level(LOG_VERBOSE);
	LOGI("Start");

	buttonSignal = new ButtonSignal();
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);

	mosqpp::lib_init();

	config = new Config();
	config->ReadConfig();

	timerSchedule = new TimerSchedule();
	timerSchedule->init();

	database = new Db();

#ifdef CONFIG_ENABLE_ZIGBEE
	zigbeeProtocol = new ZigbeeProtocol((char *)ZIGBEE_UART_PORT, B115200);
	zigbeeProtocol->init();
#endif
	string mac = Wifi::GetMacAddress();
	LOGI("mac: %s", mac.c_str());
	gateway = new Gateway(mac, config->GetHost(), config->GetPort(), mac, config->GetUsername(), config->GetPassword(), config->GetKeepAlive(), config->GetLocalHost(), config->GetLocalPort(), config->GetLocalUsername(), config->GetLocalPassword(), 10);
	gateway->init();

	bleProtocol = new BleProtocol((char *)BLE_UART_PORT, B115200);
	bleProtocol->init();

	Device::InitDeviceModelList();

	Util::LedService(true);

	while (1)
	{
		sleep(10);
	}
	LOGI("exit main");
	return 0;
}
