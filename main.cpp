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
	log_set_level(LOG_DEBUG);
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
	Device::InitDeviceModelList();
	zigbeeProtocol = new ZigbeeProtocol((char *)ZIGBEE_UART_PORT, B115200);
	zigbeeProtocol->init();
	zigbeeProtocol->CommissionFormation();
#endif

	string mac = Wifi::GetMacAddress();
	// string mac = "11:22:33:44:55:66";
	LOGI("mac: %s", mac.c_str());
	gateway = new Gateway(mac, config->GetHost(), config->GetPort(), config->GetClientId() + mac, config->GetUsername() + mac, config->GetPassword(), config->GetKeepAlive(),
												config->GetLocalHost(), config->GetLocalPort(), config->GetLocalUsername(), config->GetLocalPassword(), config->GetLocalKeepAlive());
	gateway->init();

	bleProtocol->InitKey();

	mqttProtocol = new MqttProtocol();
	mqttProtocol->init();

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
