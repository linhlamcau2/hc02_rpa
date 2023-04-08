#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "json.h"
#include "ErrorCode.h"

using namespace std;

namespace Util
{
	// string getTimeStrFromTime(time_t t);
	string getCurrentTimeStr();
	/**
	 * @brief Get the Current Timer in minute object
	 *
	 * @return int
	 */
	int GetCurrentTimer();

	int GetYearsCurrent();
	int GetMonthsCurrent();
	int GetDateCurrent();
	int GetDaysCurrent();
	int GetHoursCurrent();
	int GetMinutesCurrent();
	int GetSecondsCurrent();

	/**
	 * @brief Get the Current Time in milli seconds object
	 *
	 * @return int
	 */
	double millis();

	string genRandRQI(int size);
	string GenIdDeviceByElement(string id, int element);

	int GetCurrentWeekDay();
	int ConvertStrTimeToInt(string time);
	uint8_t CalCrc(uint8_t length, uint8_t *data);
	string setString(const char *value);
	string ConvertU32ToHexString(uint8_t *data, int len);
	int ConvertRepeatDayToInt(int mon, int tue, int wed, int thu, int fri, int sat, int sun);
	int ConvertWeekDayToIntCompare(int day);
	vector<string> splitString(string str, char splitter);
	bool CompareNumber(int a, int b, int c, string op);

	string ExecuteCMD(char const *command);
	string GetCurrentTimeStr();

	string uuidToStr(uint8_t *uuid);
	string arrayToString844412(uint8_t *array);

	void LedInternet(bool value);
	void LedService(bool value);
	void LedZigbee(bool value);
	void LedBle(bool value);
	void LedAll(bool value);
	void LedRestoreLastValue();
	void LedServiceLock();
	void LedServiceUnlock();

	bool GetStatusLedBle();
	bool GetStatusLedService();
	bool GetStatusLedZigbee();
	bool GetStatusLedInternet();
}
