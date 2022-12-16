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
	int GetCurrentTimer();
	int GetCurrentWeekDay();
	int ConvertStrTimeToInt(string time);
	uint8_t CalCrc(uint8_t length, uint8_t *data);
	string setString(const char *value);
	string ConvertU32ToHexString(uint8_t *data, int len);
	vector<string> splitString(string str, char splitter);
	bool CompareNumber(int a, int b, string op);

	string GetMacAddress();
	string GetIP();
	void ScanWifi(Json::Value &jsonValue);
	int ConnectToWifi(string ssid, string password, string encryption);
}
