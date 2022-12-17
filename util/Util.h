#pragma once

#include <string>
#include <vector>
#include "json.h"

using namespace std;

namespace Util
{
	string genRandRQI(int size);
	string getTimeStrFromTime(time_t t);
	string getCurrentTimeStr();

	/**
	 * @brief Get the Current Timer in minute object
	 *
	 * @return int
	 */
	int GetCurrentTimer();

	/**
	 * @brief Get the Current Time in milli seconds object
	 *
	 * @return int
	 */
	double millis();

	int GetCurrentWeekDay();
	int ConvertStrTimeToInt(string time);
	uint8_t CalCrc(uint8_t length, uint8_t *data);
	string setString(const char *value);
	string ConvertU32ToHexString(uint8_t *data, int len);
	vector<string> splitString(string str, char splitter);
	bool CompareNumber(int a, int b, string op);

	string ExecuteCMD(char const *command);
	string GetMacAddress();
	string GetIP();
	void ScanWifi(Json::Value &jsonValue);
	int ConnectToWifi(string ssid, string password, string encryption);

	void LedInternet(bool value);
	void LedService(bool value);
	void LedZigbee(bool value);
	void LedBle(bool value);
	void LedAll(bool value);
	void LedRestoreLastValue();
}
