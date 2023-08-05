#include "Gateway.h"
#include "Log.h"
#include "Wifi.h"
#include "Util.h"
#include "Db.h"
#include <algorithm>

#ifdef ESP_PLATFORM
#include "mongoose.h"
#endif

void Gateway::InitUdpMessage()
{
	UdpCmdCallbackRegister("SCAN_HC", bind(&Gateway::OnUdpScanHc, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("HC_SCAN_WIFI", bind(&Gateway::OnUdpHcScanWifi, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("SETUP_HC", bind(&Gateway::OnUdpHcSetup, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("HC_CONNECT_TO_CLOUD", bind(&Gateway::OnUdpHcConnectCloud, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("SET_PASSWD_MQTT_ONLINE", bind(&Gateway::OnRpcSetPwMqttOnline, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("aiHubBroadCast", bind(&Gateway::OnUdpHcInfo, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnUdpScanHc(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpScanHc");
	if (reqValue.isMember("DORMITORY_ID") && reqValue["DORMITORY_ID"].isString())
	{
		string dormitoryId = reqValue["DORMITORY_ID"].asString();
		if (this->dormitoryId != "" && this->dormitoryId != dormitoryId)
		{
			return CODE_ERROR;
		}
		string macGw = mac;
		string hostName = "";
		macGw.erase(remove_if(macGw.begin(), macGw.end(), [](char c)
													{ return c == ':'; }),
								macGw.end());
		respValue["CMD"] = "HC_RESPONSE";
		respValue["IP"] = Wifi::GetIP();
#ifdef ESP_PLATFORM
		hostName = "RD_MH_" + macGw.substr(macGw.size() - 4, 4);
		respValue["TYPE"] = 2;
#if MG_ENABLE_MBEDTLS
		respValue["TLS"] = true;
		respValue["MQTT_PORT"] = 8883;
#else
		respValue["TLS"] = false;
		respValue["MQTT_PORT"] = 1883;
#endif
#else
		hostName = "RD_HC_" + macGw.substr(macGw.size() - 4, 4);
		respValue["TYPE"] = 1;
		respValue["TLS"] = false;
		respValue["MQTT_PORT"] = 1883;
#endif
		for (auto &c : hostName)
			c = toupper(c);
		respValue["HOSTNAME"] = hostName;
		respValue["MAC"] = mac;
		respValue["VERSION"] = STR(VERSION);
		return CODE_OK;
	}
	else
	{
		LOGW("OnUdpScanHc payload: %s error", reqValue.toString().c_str());
	}
	return CODE_ERROR;
}

int Gateway::OnUdpHcScanWifi(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpHcScanWifi");
	string rqi = "";
	if (reqValue.isMember("REQUEST_ID") && reqValue["REQUEST_ID"].isString())
	{
		rqi = reqValue["REQUEST_ID"].asString();
	}
	if (reqValue.isMember("FROM") && reqValue.isMember("TO"))
	{
		Json::Value from;
		Json::Value to;
		from = reqValue["FROM"];
		to = reqValue["TO"];
		if (from.isMember("TYPE") && from["TYPE"].isInt() && to.isMember("TYPE") && to["TYPE"].isInt())
		{
			if (from["TYPE"].asInt() == 0 && to["TYPE"].asInt() == 2)
			{
				Json::Value fromRsp;
				Json::Value toRsp;
				Json::Value dataRsp;
				StopUdpBroadcast();
				respValue["CMD"] = "HC_SCAN_WIFI_RESPONSE";
				respValue["REQUEST_ID"] = rqi;
				respValue["TIME"] = Util::GetCurrentTimeStr();
				respValue["CONNECTION_TYPE"] = 0;
				fromRsp["TYPE"] = 2;
				respValue["FROM"] = fromRsp;
				toRsp["TYPE"] = 0;
				respValue["TO"] = toRsp;
				Wifi::ScanWifi(dataRsp);
				respValue["DATA"] = dataRsp;
				return CODE_OK;
			}
			else
			{
				LOGW("OnUdpHcScanWifi payload: %s error direction", reqValue.toString().c_str());
			}
		}
		else
		{
			LOGW("OnUdpHcScanWifi payload: %s error", reqValue.toString().c_str());
		}
	}
	else
	{
		LOGW("OnUdpHcScanWifi payload: %s error", reqValue.toString().c_str());
	}

	return CODE_ERROR;
}

int Gateway::OnUdpHcSetup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpHcSetup");
	string rqi = "";
	if (reqValue.isMember("REQUEST_ID") && reqValue["REQUEST_ID"].isString())
	{
		rqi = reqValue["REQUEST_ID"].asString();
	}
	if (reqValue.isMember("FROM") && reqValue.isMember("TO") && reqValue.isMember("DATA"))
	{
		Json::Value from = reqValue["FROM"];
		Json::Value to = reqValue["TO"];
		Json::Value data = reqValue["DATA"];

		string timeValue = Util::GetCurrentTimeStr();
		respValue["CMD"] = "SETUP_HC_RESPONSE";
		respValue["REQUEST_ID"] = rqi;
		respValue["TIME"] = timeValue;
		respValue["CONNECTION_TYPE"] = 0;

		Json::Value toRsp;
		Json::Value fromRsp;
		Json::Value dataRsp;

		toRsp["TYPE"] = 0;
		if (from.isMember("OS") && from["OS"].isString())
		{
			toRsp["OS"] = from["OS"].asString();
		}
		if (from.isMember("OS_VERSION") && from["OS_VERSION"].isString())
		{
			toRsp["OS_VERSION"] = from["OS_VERSION"].asString();
		}
		if (from.isMember("APP_BUILD") && from["APP_BUILD"].isString())
		{
			toRsp["APP_BUILD"] = from["APP_BUILD"].asString();
		}
		if (from.isMember("APP_VERSION") && from["APP_VERSION"].isString())
		{
			toRsp["APP_VERSION"] = from["APP_VERSION"].asString();
		}
		respValue["TO"] = toRsp;

		fromRsp["TYPE"] = 2;
		fromRsp["MAC"] = mac;
		fromRsp["VERSION"] = STR(VERSION);

		if (from.isMember("TYPE") && from["TYPE"].isInt() && to.isMember("TYPE") && to["TYPE"].isInt())
		{
			if ((from["TYPE"].asInt() == 0) && to["TYPE"].asInt())
			{
				if (data.isMember("DORMITORY_ID") && data["DORMITORY_ID"].isString())
				{
					dormitoryId = data["DORMITORY_ID"].asString();
					database->GatewayUpdateDormitory(gateway, dormitoryId);
					Json::Value jsonData;
					jsonData["data"] = Json::objectValue;
					gateway->pushStartAddHc(jsonData);
					if (Wifi::GetIP().compare("10.10.10.1") != 0)
					{
						LOGI("Hc have IP: %s", Wifi::GetIP().c_str());
						dataRsp["STATUS"] = "SUCCESS";
						respValue["DATA"] = dataRsp;
						fromRsp["IP"] = Wifi::GetIP();
						fromRsp["DORMITORY_ID"] = dormitoryId;
						respValue["FROM"] = fromRsp;
						GatewayConnectToCloudNotice();
						return CODE_OK;
					}
					else
					{
						if (data.isMember("WIFI"))
						{
							Json::Value wifi = data["WIFI"];
							if (wifi.isMember("SSID") && wifi["SSID"].isString() &&
									wifi.isMember("PASSWORD") && wifi["PASSWORD"].isString() &&
									wifi.isMember("ENCRYPTION") && wifi["ENCRYPTION"].isString())
							{
								string ssid = wifi["SSID"].asString();
								string password = wifi["PASSWORD"].asString();
								string encryption = wifi["ENCRYPTION"].asString();
								LOGD("ssid: %s, password: %s, encryption: %s", ssid.c_str(), password.c_str(), encryption.c_str());

								if (Wifi::ConnectToWifi(ssid, password, encryption) == 0)
								{
									dataRsp["STATUS"] = "SUCCESS";
									respValue["DATA"] = dataRsp;
									fromRsp["IP"] = Wifi::GetIP();
									fromRsp["DORMITORY_ID"] = dormitoryId;
									respValue["FROM"] = fromRsp;
								}
								else
								{
									dataRsp["STATUS"] = "FAILED";
									respValue["DATA"] = dataRsp;
									fromRsp["IP"] = Wifi::GetIP();
									fromRsp["DORMITORY_ID"] = dormitoryId;
									respValue["FROM"] = fromRsp;
								}
								GatewayConnectToCloudNotice();
								Json::Value jsonData;
								jsonData["data"] = Json::objectValue;
								gateway->pushStopAddHc(jsonData);
								return CODE_OK;
							}
							else
							{
								LOGW("OnUdpHcSetup don't have wifi data");
							}
						}
						else
						{
							LOGW("OnUdpHcSetup don't have wifi object");
						}
					}
				}
				else
				{
					LOGW("OnUdpHcSetup don't have dormitory");
				}
			}
			else
			{
				LOGW("OnUdpHcSetup error data from - to");
			}
		}
		else
		{
			LOGW("OnUdpHcSetup don't have from or to");
		}
	}
	else
	{
		LOGW("OnUdpHcSetup payload: %s error", reqValue.toString().c_str());
	}
	return CODE_ERROR;
}

int Gateway::OnUdpHcConnectCloud(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpHcConnectCloud");
	return CODE_ERROR;
}

int Gateway::OnUdpHcInfo(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcRspHcInfo");
	Json::Value dataValue;
	dataValue["mac"] = mac;
	dataValue["ip"] = Wifi::GetIP();
	dataValue["name"] = "RD HC";
	dataValue["type"] = MODEL;
	dataValue["ver"] = STR(VERSION);
	respValue["data"] = dataValue;
	respValue["cmd"] = "getHcInfoRsp";
	return CODE_OK;
}