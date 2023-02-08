#include "Wifi.h"
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
#include "Log.h"
#include "Util.h"

#include "Gateway.h"

void Wifi::InitWifi()
{
	// ESP_ERROR_CHECK(esp_netif_init());
	// ESP_ERROR_CHECK(esp_event_loop_create_default());

	// sta = esp_netif_create_default_wifi_sta();
	// ap = esp_netif_create_default_wifi_ap();

	// wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	// ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	// ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
	// ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

	// esp_netif_ip_info_t ipInfo;
	// IP4_ADDR(&ipInfo.ip, 10, 10, 10, 1);
	// IP4_ADDR(&ipInfo.gw, 10, 10, 10, 1);
	// IP4_ADDR(&ipInfo.netmask, 255, 255, 255, 0);
	// esp_netif_dhcps_stop(ap);
	// esp_netif_set_ip_info(ap, &ipInfo);
	// esp_netif_dhcps_start(ap);

	// wifi_config_t wifi_config;
	// esp_err_t ret = esp_wifi_get_config(WIFI_IF_STA, &wifi_config);
	// if (ret == ESP_OK)
	// {
	// 	ESP_LOGI(TAG, "Wifi configuration already stored in flash partition called NVS");
	// 	ESP_LOGI(TAG, "%s", wifi_config.sta.ssid);
	// 	ESP_LOGI(TAG, "%s", wifi_config.sta.password);
	// 	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	// 	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	// 	ESP_ERROR_CHECK(esp_wifi_start());
	// 	wifiMode = WIFI_MODE_STA;
	// }
	// else
	// {
	// 	ESP_LOGI(TAG, "Wifi configuration not found in flash partition called NVS.");
	// }
}
string Wifi::GetMacAddress()
{
	LOGD("GetMacAddress");
	struct ifreq s;
	unsigned char *mac = NULL;
	char uc_Mac[100];
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	strcpy(s.ifr_name, "eth0");
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s))
	{
		mac = (unsigned char *)s.ifr_addr.sa_data;
	}
	sprintf((char *)uc_Mac, (const char *)"%.2x%.2x%.2x%.2x%.2x%.2x",
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return string(uc_Mac);
}

string Wifi::GetIP()
{
	LOGD("GetIP");
	string string_ip_1;
	string string_ip_2;
	string string_ip_3;
	string msg_rsp = Util::ExecuteCMD("ip -4 addr show ${link_name} | sed -Ene \'s/^.*inet ([0-9.]+)\\/.*$/\\1/p\'");
	// LOGV("IP list: %s", msg_rsp.c_str());
	vector<string> ipList = Util::splitString(msg_rsp, '\n');
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

void Wifi::ScanWifi(Json::Value &data)
{
	size_t pos = 0;
	string wifi;
	vector<string> wifiDataList;
	char buff[128] = "Cell 02";
	int count = 0;
	vector<string> lineList;
	string ssidStr, encryptionStr;
	int quality;

	string msg_rsp = Util::ExecuteCMD("iwinfo wlan0 scan");
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
		lineList = Util::splitString(wifiData, '\n');
		if (lineList.size() == 5 && (lineList[1].find("\"") != string::npos))
		{
			ssidStr = trim(lineList[1]);
			ssidStr.erase(0, 8);
			ssidStr.erase(ssidStr.length() - 1);

			encryptionStr = trim(lineList[4]);
			encryptionStr.erase(0, 12);

			char *p;
			p = (char *)strstr(lineList[3].c_str(), "Quality");
			char qlt[3] = {(*(p + 9)), *(p + 10)};
			quality = stoi(string(qlt));
			if (quality >= 50)
			{
				Json::Value wifiValue;
				wifiValue["SSID"] = ssidStr;
				wifiValue["QUALITY"] = quality;
				wifiValue["MAC"] = lineList[0];
				wifiValue["ENCRYPTION"] = encryptionStr;
				data.append(wifiValue);
			}
		}
	}
}

int Wifi::ConnectToWifi(string ssid, string password, string encryption)
{
	LOGD("Connect to Wifi");
	if (encryption == "WPA2 PSK (CCMP)")
	{
		encryption = "psk2";
	}
	else if (encryption == "none")
	{
		encryption = "psk2";
	}
	else if (encryption == "WPA PSK")
	{
		encryption = "psk";
	}
	try
	{
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
	}
	catch (...)
	{
	}
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

int Wifi::SetModeApWifi()
{
	system("rm /output.txt");
	system("uci set network.wan.ifname='eth0' >> /output.txt 2>&1");
	system("uci commit network >> /output.txt 2>&1");
	system("/etc/init.d/network restart >> /output.txt 2>&1");
	system("uci del wireless.wifinet1 >> /output.txt 2>&1");
	system("uci commit wireless >> /output.txt 2>&1");
	system("wifi >> /output.txt 2>&1");
	return 0;
}

bool Wifi::WifiIsAPMode(void)
{
	return false;
	// return wifiMode == WIFI_MODE_APSTA;
}

bool Wifi::WifiIsStaMode(void)
{
	return false;
	// return wifiMode == WIFI_MODE_AP;
}
