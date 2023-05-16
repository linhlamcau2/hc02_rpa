#include "DeviceBleScreenTouch.h"
#include <thread>
#include "BleProtocol.h"
#include "Util.h"
#include "Log.h"
#include "Http.h"

#ifdef ESP_PLATFORM
#include "Led.h"
#endif

static void SendDatetime(void *data);

DeviceBleScreenTouch::DeviceBleScreenTouch(string id, string name, string mac, string data, uint32_t addr, uint16_t version)
	: DeviceBle(id, name, mac, data, addr, BLE_AC_SCENE_SCREEN_TOUCH, version)
{
	moduleNotifyScene = new ModuleNotifyScene(this, addr);
	modules.push_back(moduleNotifyScene);
	powerSource = POWER_AC;

#ifdef ESP_PLATFORM
	LOGI("Free memory: %d bytes, internal: %d bytes", esp_get_free_heap_size(), esp_get_free_internal_heap_size());
	if (xTaskCreate(SendDatetime, "SendDatetime", 5120, this, 10, NULL) != pdPASS)
	{
		LOGE("Failed to create task");
		Led::SetLedService(MODE_OFF);
	}
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
	string ST_array_icon[18] = {"01d", "02d", "03d", "04d", "09d", "10d", "11d",
								"13d", "50d", "01n", "02n", "03n", "04n", "09n", "10n", "11n", "13n",
								"50n"};
	while (1)
	{
		if (bleProtocol)
		{
			bleProtocol->SendDate(deviceBleScreenTouch->GetAddr(), Util::GetYearsCurrent(), Util::GetMonthsCurrent(), Util::GetDateCurrent(), Util::GetDaysCurrent());
			bleProtocol->SendTime(deviceBleScreenTouch->GetAddr(), Util::GetHoursCurrent(), Util::GetMinutesCurrent(), Util::GetSecondsCurrent());

			HTTPRequest *httpRequest = new HTTPRequest();
			httpRequest->setUrl(string(BASE_URL_DEV) + string(RENEW_TOKEN));
			httpRequest->setMethod("GET");
			string dataWeather = httpRequest->GetWeather(Util::GetLongitude(), Util::GetLatitude());
			LOGW("dataWeather:%s", dataWeather.c_str());
			Json::Value dataWeatherJson;
			if (dataWeatherJson.parse(dataWeather) && dataWeatherJson.isObject())
			{
				if (dataWeatherJson.isMember("weather") && dataWeatherJson["weather"].isArray() &&
					dataWeatherJson.isMember("main") && dataWeatherJson["main"].isObject())
				{
					Json::Value weather = dataWeatherJson["weather"][0];
					Json::Value main = dataWeatherJson["main"];
					if (weather.isMember("icon") && weather["icon"].isString() && main.isMember("temp") && main["temp"].isDouble())
					{
						string icon = weather["icon"].asString();
						int StatusWeather = 0;
						for (int i = 0; i < 17; i++)
						{
							if (icon.compare(ST_array_icon[i]) == 0)
							{
								StatusWeather = i;
								break;
							}
						}
						uint16_t temp = main["temp"].asInt();
						bleProtocol->SendWeatherOutdoor(deviceBleScreenTouch->GetAddr(), StatusWeather, temp);
					}
				}
			}
			delete httpRequest;
		}
		else
			LOGW("BleProtocol null");
		sleep(1800);
	}
}
