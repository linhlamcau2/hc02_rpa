#include "ButtonSignal.h"
#include <unistd.h>
#include <stdio.h>
#include "Log.h"
#include "Util.h"
#include "Gateway.h"

#define DOUBLE_CLICK_TIME 400
#define AP_MODE_WIFI	  5

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
	while (isBlinkLed && blinkCount < 7)
	{
		blinkCount++;
		Util::LedAll(false);
		usleep(500000);
		Util::LedAll(true);
		usleep(500000);
	}
	Util::LedRestoreLastValue();
}

void ButtonSignal::OnRelease()
{
	LOGI("OnRelease");
	isBlinkLed = false;
	releaseTime = Util::millis();
	LOGD("interval: %d", (releaseTime - pressTime));
	if (releaseTime - pressTime < DOUBLE_CLICK_TIME)
	{
		clickCount++;
		LOGI("clickCount: %d", clickCount);
		if(clickCount == AP_MODE_WIFI)
		{
			LOGI("set AP mode wifi");
			Util::SetModeApWifi();
		}
	}
	else if (startProcess)
	{
		if (releaseTime - pressTime > 5000 && releaseTime - pressTime < 8000)
		{
			gateway->StartUdpBroadcast();
		}
	}
}