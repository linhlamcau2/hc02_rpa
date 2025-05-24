#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <signal.h>
#include <fstream>
#include "json.h"
#include "Log.h"
#include "Config.h"
#include "Gateway.h"
#include "Device.h"
#include "Util.h"
#include "Wifi.h"
#include "ButtonSignal.h"
#include "BleProtocol.h"
#include "QrProtocol.h"
#include "RelayProtocol.h"

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
	LOGI("Start ver " STR(VERSION));

	buttonSignal = new ButtonSignal();
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);

	srand(time(0));

	mosqpp::lib_init();

	config = new Config();
	config->ReadConfig();

	Util::ExecuteCMD("echo 0 > /sys/class/gpio/export");
	
	bleProtocol = new BleProtocol((char *)BLE_UART_PORT, B115200);
	bleProtocol->init();

	qrProtocol = new QrProtocol((char *)QR_UART_PORT, B115200);
	qrProtocol->init();

	relayProtocol = new RelayProtocol((char *)RELAY_UART_PORT, B115200);
	relayProtocol->init();

	Util::LedInternet(false);

	string mac = Wifi::GetMacAddress();
	// string mac = "11:22:33:44:55:66";
	LOGI("mac: %s", mac.c_str());

	string passMqttLocal = "1";
#ifndef ESP_PLATFORM
	string macNotDot = mac;
	macNotDot.erase(std::remove(macNotDot.begin(), macNotDot.end(), ':'), macNotDot.end());
	string key = "RANGDONGRALSMART";
	string plainText = "2804" + macNotDot;
	passMqttLocal = Util::encryptAes128(key, plainText);
	for (char &c : passMqttLocal)
	{
		c = std::toupper(c);
	}
	LOGI("Passsword: %s", passMqttLocal.c_str());
#endif
	gateway = new Gateway(mac, config->GetHost(), config->GetPort(), config->GetClientId(), config->GetUsername(), config->GetPassword(), config->GetKeepAlive(),
						  "localhost", 1883, "RD", passMqttLocal, 10);
	gateway->init();

	bleProtocol->StopScan();

	Util::LedService(true);
	Util::LedZigbee(false);

	while (1)
	{
		sleep(10);
	}
	LOGI("exit main");
	return 0;
}
