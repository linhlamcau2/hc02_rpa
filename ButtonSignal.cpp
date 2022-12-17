#include "ButtonSignal.h"
#include <unistd.h>
#include <stdio.h>
#include "Log.h"
#include "Util.h"
#include "Gateway.h"

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
	if (pressTime - releaseTime > 500)
	{
		startProcess = true;
		clickCount = 0;
	}
	isBlinkLed = true;
	sleep(1);
	while (isBlinkLed && blinkCount < 7)
	{
		blinkCount++;
		Util::LedAll(0);
		usleep(800000);
		Util::LedAll(1);
		usleep(200000);
	}
}

void ButtonSignal::OnRelease()
{
	LOGI("OnRelease");
	isBlinkLed = false;
	releaseTime = Util::millis();
	if (releaseTime - pressTime < 500)
	{
		clickCount++;
		LOGI("clickCount: %d", clickCount);
	}
	else if (startProcess)
	{
		if (releaseTime - pressTime > 5000 && releaseTime - pressTime < 7000)
		{
			gateway->StartUdpBroadcast();
		}
	}
}