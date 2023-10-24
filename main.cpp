#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <signal.h>
#include "json.h"
#include "Log.h"
#include "Config.h"
#include "Gateway.h"
#include "Device.h"
#include "Db.h"
#include "Util.h"
#include "Wifi.h"
#include "TimerSchedule.h"
#include "ButtonSignal.h"
#include "BleProtocol.h"
#include "MqttProtocol.h"
#include "AndroidBleProtocol.h"

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
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
#ifndef __ANDROID__
	log_set_level(LOG_DEBUG);
#endif
	LOGI("Start");

	buttonSignal = new ButtonSignal();
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);

	srand(time(0));

	mosqpp::lib_init();

	config = new Config();
	config->ReadConfig();

	timerSchedule = new TimerSchedule();
	timerSchedule->init();

	database = new Db();
	database->init();

	bleProtocol = new BleProtocol((char *)BLE_UART_PORT, B115200);
	bleProtocol->init();

#ifdef CONFIG_ENABLE_ZIGBEE
	zigbeeProtocol = new ZigbeeProtocol((char *)ZIGBEE_UART_PORT, B115200);
	zigbeeProtocol->init();
	// zigbeeProtocol->CommissionFormation();
#endif

	string mac = Wifi::GetMacAddress();
	// string mac = "be:3e:25:2e:a1:ca";
	LOGI("mac: %s", mac.c_str());
	gateway = new Gateway(mac, config->GetHost(), config->GetPort(), "hc-" + mac, "hc-" + mac, config->GetPassword(), config->GetKeepAlive(),
												"localhost", 1883, "RD", "", 10);
	gateway->init();

	bleProtocol->InitKey();

	mqttProtocol = new MqttProtocol();
	mqttProtocol->init();
	
	Device::InitDeviceModelList();

	androidBleProtocol = new AndroidBleProtocol();
	androidBleProtocol->init();

	Util::LedService(true);

	// fileTransfer = new FileTransfer();
	// fileTransfer->init();
	// fileTransfer->uploadFile(".", "smh.sqlite");
	// fileTransfer->uploadFile(".", "readme.txt");
	// thread sendFile1(bind(&FileTransfer::uploadFile, fileTransfer, ".", "osiot1.rar"));
	// sendFile1.detach();
	// thread sendFile2(bind(&FileTransfer::uploadFile, fileTransfer, ".", "osiot2.rar"));
	// sendFile2.detach();

	while (1)
	{
		sleep(10);
	}
	LOGI("exit main");
	return 0;
}
