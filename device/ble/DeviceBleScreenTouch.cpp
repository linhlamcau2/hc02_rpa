#include "DeviceBleScreenTouch.h"
#include <thread>
#include "BleProtocol.h"
#include "Util.h"
#include "Log.h"

static void SendDatetime(void *data);

DeviceBleScreenTouch::DeviceBleScreenTouch(string id, string name, string mac, string data, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, data, addr, BLE_AC_SCENE_SCREEN_TOUCH, version)
{
	moduleNotifyScene = new ModuleNotifyScene(this, addr);
	modules.push_back(moduleNotifyScene);
	powerSource = POWER_AC;

#ifdef ESP_PLATFORM
	xTaskCreate(SendDatetime, "SendDatetime", 5120, this, 10, NULL);
	vTaskDelay(10);
#else
	thread SendDatetimeThread(SendDatetime, this);
	SendDatetimeThread.detach();
#endif
}

static void SendDatetime(void *data)
{
	LOGD("SendDatetime Start");
	DeviceBleScreenTouch *deviceBleScreenTouch = (DeviceBleScreenTouch *)data;
	while (1)
	{
		if (bleProtocol)
		{
			bleProtocol->SendDate(deviceBleScreenTouch->GetAddr(), Util::GetYearsCurrent(), Util::GetMonthsCurrent(), Util::GetDateCurrent(), Util::GetDaysCurrent());
			bleProtocol->SendTime(deviceBleScreenTouch->GetAddr(), Util::GetHoursCurrent(), Util::GetMinutesCurrent(), Util::GetSecondsCurrent());
		}
		else
			LOGW("BleProtocol null");
		sleep(3600);
	}
}
