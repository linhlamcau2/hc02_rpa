#include "ButtonSignal.h"
#include <unistd.h>
#include <stdio.h>
#include "Log.h"
#include "Util.h"
#include "Wifi.h"
#include "Gateway.h"

#define DOUBLE_CLICK_TIME 400
#define AP_MODE_WIFI 5
#define MODE_SEND_UDP_BROADCAST 3

ButtonSignal *buttonSignal = NULL;

ButtonSignal::ButtonSignal()
{
	pressTime = 0;
	releaseTime = 0;
	startProcess = true;
	clickCount = 0;
}

void ButtonSignal::OnPress()
{
	LOGI("OnPress");
	int blinkCount = 0;
	pressTime = Util::millis();
	if (pressTime - releaseTime > DOUBLE_CLICK_TIME)
	{
		startProcess = true;
		clickCount = 0;
	}
	isBlinkLed = true;
	sleep(1);
	while (isBlinkLed /*&& blinkCount < 7*/)
	{
		// blinkCount++;
		Util::LedAll(false);
		usleep(490000);
		Util::LedAll(true);
		usleep(490000);
	}
	Util::LedRestoreLastValue();
}

void ButtonSignal::OnRelease()
{
	LOGI("OnRelease");
	isBlinkLed = false;
	releaseTime = Util::millis();
	if (releaseTime - pressTime < DOUBLE_CLICK_TIME)
	{
		clickCount++;
		LOGI("clickCount: %d", clickCount);
		if (clickCount == MODE_SEND_UDP_BROADCAST)
		{
			// gateway->StartUdpBroadcast();
		}
	}
	else if (startProcess)
	{
		if (releaseTime - pressTime > 1000 && releaseTime - pressTime < 5000)
		{
			bleProtocol->ResetBle();
		}
		else if (releaseTime - pressTime > 6000 && releaseTime - pressTime < 9000)
		{
			LOGW("set AP mode wifi");
			Wifi::SetModeApWifi();
		}
		else if (releaseTime - pressTime > 12000 && releaseTime - pressTime < 15000)
		{
			LOGW("Reset Dormitory");
			gateway->setDormitory("");
		}
		else if (releaseTime - pressTime > 20000 && releaseTime - pressTime < 23000)
		{
			LOGW("Reset HC");
			gateway->ResetFactory();
			Wifi::SetModeApWifi();
			exit(1);
		}
	}
}
