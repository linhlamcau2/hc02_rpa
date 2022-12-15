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
#include "TimerSchedule.h"

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
		if (gateway)
		{
			gateway->StartUdpBroadcast();
		}
	}
	signal(sig, signal_handler);
}

int main(int argc, char *argv[])
{
	signal(SIGUSR1, signal_handler);

	log_set_level(LOG_VERBOSE);
	LOGI("Start");

	config = new Config();
	config->ReadConfig();

	timerSchedule = new TimerSchedule();
	timerSchedule->init();

	bleProtocol = new BleProtocol((char *)BLE_UART_PORT, B115200);
	bleProtocol->init();
	
#ifdef CONFIG_ENABLE_ZIGBEE
	zigbeeProtocol = new ZigbeeProtocol((char *)ZIGBEE_UART_PORT, B115200);
	zigbeeProtocol->init();
#endif

	database = new Db();
	gateway = new Gateway(config->GetHost(), config->GetPort(), config->GetClientId() + to_string(rand()), config->GetUsername(), config->GetPassword(), config->GetKeepAlive());
	gateway->init();

	Device::InitDeviceModelList();

	while (1)
	{
		sleep(10);
	}
	LOGI("exit main");
	return 0;
}
