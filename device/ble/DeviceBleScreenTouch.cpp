#include "DeviceBleScreenTouch.h"
#include <thread>
#include "BleProtocol.h"
#include "Util.h"
#include "Log.h"

DeviceBleScreenTouch::DeviceBleScreenTouch(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_AC_SCENE_SCREEN_TOUCH, version)
{
	thread sendDateTimeThread(bind(&DeviceBleScreenTouch::SendDatetime, this));
	sendDateTimeThread.detach();
	powerSource = POWER_AC;
}

void DeviceBleScreenTouch::SendDatetime()
{
	while (1)
	{
		if (bleProtocol)
		{
			bleProtocol->SendDate(addr, Util::GetYearsCurrent(), Util::GetMonthsCurrent(), Util::GetDateCurrent(), Util::GetDaysCurrent());
			bleProtocol->SendTime(addr, Util::GetHoursCurrent(), Util::GetMinutesCurrent(), Util::GetSecondsCurrent());
		}
		sleep(3600);
	}
}
