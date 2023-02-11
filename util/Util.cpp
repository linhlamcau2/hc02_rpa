#include "Util.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
// #include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
// #include <linux/if.h>
#include <sys/socket.h>
#include <algorithm>
#include <functional>
#include <cctype>
#include <ctime>
#include <locale>
#include <Base64.h>
#include "Log.h"

using namespace std;

string Util::genRandRQI(int size)
{
	string rqi = "";
	for (int i = 0; i < size; i++)
	{
		rqi += 'a' + rand() % 26;
	}
	return rqi;
}

string getTimeStrFromTime(time_t t)
{
	struct tm start;
	start = *localtime(&t);
	char timeBuffer[100];
	sprintf(timeBuffer, "%04d%02d%02dT%02d%02d%02d", start.tm_year + 1900, start.tm_mon + 1, start.tm_mday, start.tm_hour, start.tm_min, start.tm_sec);
	return string(timeBuffer);
}

string Util::GetCurrentTimeStr()
{
	return getTimeStrFromTime(time(NULL));
}

int Util::GetCurrentTimer()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_hour * 60 + lt.tm_min;
}

double Util::millis()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

int Util::GetCurrentWeekDay()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_wday;
}

int Util::ConvertStrTimeToInt(string time)
{
	int hour;
	int minute;
	if (sscanf(time.c_str(), "%d:%d", &hour, &minute) == 2)
		return hour * 60 + minute;
	return -1;
}

uint8_t Util::CalCrc(uint8_t length, uint8_t *data)
{
	uint8_t crc = 0;
	int i = 0;
	for (i = 0; i < length; i++)
	{
		crc = crc ^ data[i];
	}
	crc = crc & 0xFF;
	return crc;
}

string Util::setString(const char *value)
{
	return value ? value : "";
}

string Util::ConvertU32ToHexString(uint8_t *data, int len)
{
	char buff[100];
	if (len > 50)
		len = 50;
	for (int i = 0; i < len; i++)
	{
		sprintf(buff + i * 2, "%02X", data[i]);
	}
	return string(buff);
}

int Util::ConvertRepeatDayToInt(int mon, int tue, int wed, int thu, int fri, int sat, int sun)
{
	return mon * 64 + tue * 32 + wed * 16 + thu * 8 + fri * 4 + sat * 2 + sun;
}

vector<string> Util::splitString(string str, char splitter)
{
	vector<string> result;
	string current = "";
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == splitter)
		{
			if (current != "")
			{
				result.push_back(current);
				current = "";
			}
			continue;
		}
		current += str[i];
	}
	if (current.size() != 0)
		result.push_back(current);
	return result;
}

bool Util::CompareNumber(int a, int b, string op)
{
	if (op == "==")
		return a == b;
	else if (op == "!=")
		return a != b;
	else if (op == ">")
		return a > b;
	else if (op == ">=")
		return a >= b;
	else if (op == "<")
		return a < b;
	else if (op == "<=")
		return a <= b;
	return false;
}

string Util::ExecuteCMD(char const *command)
{
	string msg_rsp = "";
#ifndef ESP_PLATFORM
	FILE *file;
	char msg_line[100] = {0};
	file = popen(command, "r");
	if (file == NULL)
	{
		exit(1);
	}
	fgets(msg_line, 100, file);
	msg_rsp += msg_line;
	while (1)
	{
		fgets(msg_line, 100, file);
		if (feof(file))
		{
			break;
		}
		msg_rsp += msg_line;
	}
	pclose(file);
#endif
	return msg_rsp;
}

static bool ledInternet = false;
static bool ledService = false;
static bool ledZigbee = false;
static bool ledBle = false;

void Util::LedInternet(bool value)
{
	ledInternet = value;
	if (value)
	{
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:internet/brightness");
	}
	else
	{
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:internet/brightness");
	}
}

void Util::LedService(bool value)
{
	ledService = value;
	if (value)
	{
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:service/brightness");
	}
	else
	{
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:service/brightness");
	}
}

void Util::LedZigbee(bool value)
{
	ledZigbee = value;
	if (value)
	{
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness");
	}
	else
	{
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness");
	}
}

void Util::LedBle(bool value)
{
	ledBle = value;
	if (value)
	{
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness");
	}
	else
	{
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness");
	}
}

void Util::LedAll(bool value)
{
	if (value)
	{
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:internet/brightness");
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:service/brightness");
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness");
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness");
	}
	else
	{
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:internet/brightness");
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:service/brightness");
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness");
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness");
	}
}

void Util::LedRestoreLastValue()
{
	LedInternet(ledInternet);
	LedService(ledService);
	LedZigbee(ledZigbee);
	LedBle(ledBle);
}

static int ledServiceCount = 0;
void Util::LedServiceLock()
{
	ledServiceCount++;
	if (ledServiceCount)
		LedService(false);
}

void Util::LedServiceUnlock()
{
	ledServiceCount--;
	if (!ledServiceCount)
		LedService(true);
}

bool Util::GetStatusLedBle()
{
	return ledBle;
}

bool Util::GetStatusLedService()
{
	return ledService;
}

bool Util::GetStatusLedZigbee()
{
	return ledZigbee;
}

bool Util::GetStatusLedInternet()
{
	return ledInternet;
}
