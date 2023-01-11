#include "Util.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <sys/socket.h>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <Base64.h>
#include "Log.h"

using namespace std;

string genRandRQI(int size)
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

string getCurrentTimeStr()
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

int Util::ConvertRepeatDayToInt(int mon, int tue, int wed, int thu, int fri, int sat, int sun)
{
	return mon * 64 + tue * 32 + wed * 16 + thu * 8 + fri * 4 + sat * 2 + sun;
}

// trim from start
static inline std::string &ltrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string &rtrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string &trim(std::string &s)
{
	return ltrim(rtrim(s));
}

string Util::ExecuteCMD(char const *command)
{
	FILE *file;
	string msg_rsp = "";
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
	return msg_rsp;
}

string Util::GetMacAddress()
{
	LOGD("GetMacAddress");
	struct ifreq s;
	unsigned char *mac = NULL;
	char uc_Mac[100];
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	strcpy(s.ifr_name, "wlx0c8c24d05f06");
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s))
	{
		mac = (unsigned char *)s.ifr_addr.sa_data;
	}
	sprintf((char *)uc_Mac, (const char *)"%.2x%.2x%.2x%.2x%.2x%.2x",
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	std::cout << uc_Mac << std::endl;
	return string(uc_Mac);
}

string Util::GetIP()
{
	LOGD("GetIP");
	string string_ip_1;
	string string_ip_2;
	string string_ip_3;
	string msg_rsp = ExecuteCMD("ip -4 addr show ${link_name} | sed -Ene \'s/^.*inet ([0-9.]+)\\/.*$/\\1/p\'");
	// LOGV("IP list: %s", msg_rsp.c_str());
	vector<string> ipList = splitString(msg_rsp, '\n');
	for (size_t i = 0; i < ipList.size(); i++)
	{
		if (ipList.at(i) != "127.0.0.1" && ipList.at(i) != "10.10.10.1")
		{
			return ipList.at(i);
		}
	}
	for (size_t i = 0; i < ipList.size(); i++)
	{
		if (ipList.at(i) != "127.0.0.1" && ipList.at(i) != "")
		{
			return ipList.at(i);
		}
	}
	return "";
}

void Util::ScanWifi(Json::Value &jsonValue)
{
	size_t pos = 0;
	string wifi;
	vector<string> wifiDataList;
	char buff[128] = "Cell 02";
	int count = 0;
	Json::Value wifiValue;
	vector<string> lineList;
	string ssidStr, encryptionStr;

	string msg_rsp = ExecuteCMD("iwinfo wlan0 scan");
	while ((pos = msg_rsp.find(buff)) != string::npos)
	{
		wifi = msg_rsp.substr(0, pos);
		msg_rsp.erase(0, pos + 19);
		wifiDataList.push_back(wifi);
		++count;
		sprintf(buff, "Cell %02d", count + 2);
	}
	for (auto &wifiData : wifiDataList)
	{
		lineList = splitString(wifiData, '\n');
		if (lineList.size() == 5 && (lineList[1].find("\"") != string::npos))
		{
			ssidStr = trim(lineList[1]);
			ssidStr.erase(0, 8);
			ssidStr.erase(ssidStr.length() - 1);

			encryptionStr = trim(lineList[4]);
			encryptionStr.erase(0, 12);

			wifiValue["CMD"] = "HC_RESPONE";
			wifiValue["SSID"] = macaron::Base64::Encode(ssidStr);
			wifiValue["QUALITY"] = 50;
			wifiValue["MAC"] = lineList[0];
			wifiValue["ENCRYPTION"] = encryptionStr;
			jsonValue.append(wifiValue);
		}
	}
}

int Util::ConnectToWifi(string ssid, string password, string encryption)
{
	LOGD("Connect to Wifi");
	if (encryption != "none")
		encryption = "psk2";
	try{
		system("rm /output.txt");
		system("uci del network.wan.ifname >> /output.txt 2>&1");
		system("uci del wireless.wifinet1  >> /output.txt 2>&1");
		system("uci set wireless.wifinet1=wifi-iface >> /output.txt 2>&1");
		system(string("uci set wireless.wifinet1.ssid=\"" + ssid + "\" >> /output.txt 2>&1").c_str());
		system("uci set wireless.wifinet1.mode='sta' >> /output.txt 2>&1");
		system("uci set wireless.wifinet1.network='wan' >> /output.txt 2>&1");
		system("uci set wireless.wifinet1.device='radio0' >> /output.txt 2>&1");
		system(string("uci set wireless.wifinet1.key='" + password + "' >> /output.txt 2>&1").c_str());
		system(string("uci set wireless.wifinet1.encryption='" + encryption + "' >> /output.txt 2>&1").c_str());
		system("uci commit wireless");
		system("wifi");
	}catch(std::exception){}
	sleep(30);
	string ip = GetIP();
	LOGI("GW ip: %s", GetIP().c_str());
	if (ip == "10.10.10.1")
	{
		system("uci del wireless.wifinet1 >> /output.txt 2>&1");
		system("uci set wireless.wifinet1=wifi-iface >> /output.txt 2>&1");
		// ExecuteCMD("uci set wireless.default_radio0.mode='ap'");
		system("uci commit wireless");
		system("uci commit network");
		system("wifi");
		system("/etc/init.d/network restart");
		return -1;
	}
	return 0;
}

int Util::SetModeApWifi()
{
	system("rm /output.txt");
	system("uci set network.wan.ifname='eth0' >> /output.txt 2>&1");
    system("uci commit network >> /output.txt 2>&1");
    system("/etc/init.d/network restart >> /output.txt 2>&1");
    system("uci del wireless.wifinet1 >> /output.txt 2>&1");
    system("uci commit wireless >> /output.txt 2>&1");
    system("wifi >> /output.txt 2>&1");
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
