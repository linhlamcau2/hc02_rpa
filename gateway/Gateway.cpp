#include "Gateway.h"
#include <Log.h>
#include <unistd.h>
#include <stdio.h>
#include <algorithm>
#include <json.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <thread>
#include "Db.h"
#include "Util.h"
#include "Base64.h"

#include "RuleInputTimer.h"
#include "RuleOutputGroup.h"
#include "RuleOutputDevice.h"

#include "BleProtocol.h"
#include "DeviceBleDownLightSmt.h"
#include "DeviceBleDownLightCobTrangTri.h"
#include "DeviceBleDownLightCobGocRong.h"
#include "DeviceBleDownLightRgbCw.h"
#include "DeviceBleBulb.h"
#include "DeviceBleChieuGuong.h"
#include "DeviceBleChieuTranh.h"
#include "DeviceBleDenBan.h"
#include "DeviceBleDownLightCobGocHep.h"
#include "DeviceBleFlood.h"
#include "DeviceBleLedDayLinear.h"
#include "DeviceBleLedDayRgbCw.h"
#include "DeviceBleLedDayRgb.h"
#include "DeviceBleOpTran.h"
#include "DeviceBleOpTranLoa.h"
#include "DeviceBleOpTuong.h"
#include "DeviceBlePanelTron.h"
#include "DeviceBlePanelVuong.h"
#include "DeviceBleThaTran.h"
#include "DeviceBleTrackLight.h"
#include "DeviceBleTubeM16.h"
#include "DeviceBleSwitch4.h"
#include "DeviceBleDCSceneContact.h"
#include "DeviceBleTempHumSensor.h"

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#include "DeviceZigbeeOnoff.h"
#include "DeviceZigbeeTelinkOnoff.h"
#endif

#ifndef VERSION
#define VERSION 0.0.1
#endif

#define STR_(x) #x
#define STR(x) STR_(x)

Gateway *gateway = NULL;

Gateway::Gateway(string mac, string server_address, int server_port, string token, string username, string password, int keepalive, string localIp, int localPort, string localUsername, string localPassword, int localKeepalive)
	: CloudProtocol(mac, server_address, server_port, token, username, password, keepalive),
	  LocalProtocol(mac, localIp, localPort, mac, localUsername, localPassword, localKeepalive),
	  Udp(8181)
{
	this->mac = mac;
	this->id = "";
	this->dormitoryId = "";
	this->ble_appkey = "";
	this->ble_appkey = "";
	this->ble_devicekey = "";
	udpBroadcastThread = NULL;
}

void Gateway::init()
{
	mosqpp::lib_init();
	CloudProtocol::init();
	LocalProtocol::init();
	Udp::init();

	LOGI("DeviceRead");
	database->GatewayRead();
	database->DeviceRead();
	database->DeviceAttributeRead();
	database->GroupRead();
	database->DeviceInGroupRead();
	database->RuleRead();

	if(gateway->getId().compare("") == 0)
	{
		id = mac;
		gateway->setId(id);
		database->GatewayUpdateId(gateway,id);
		database->GatewayRead();
	}

	UdpCmdCallbackRegister("SCAN_HC", bind(&Gateway::OnUdpScanHc, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("HC_SCAN_WIFI", bind(&Gateway::OnUdpHcScanWifi, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("SETUP_HC", bind(&Gateway::OnUdpHcSetup, this, placeholders::_1, placeholders::_2));
	UdpCmdCallbackRegister("HC_CONNECT_TO_CLOUD", bind(&Gateway::OnUdpHcConnectCloud, this, placeholders::_1, placeholders::_2));

	OnDeviceRPCCallbackRegister("SCAN", bind(&Gateway::OnRPCBleStartScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("STOP", bind(&Gateway::OnRPCBleStopScan, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("BleResetFactory", bind(&Gateway::OnRPCBleResetFactory, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("BleAddDevice", bind(&Gateway::OnRPCBleAddDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("RESET_NODE", bind(&Gateway::OnRPCBleDelDevice, this, placeholders::_1, placeholders::_2));

	OnDeviceRPCCallbackRegister("CREATE_GROUP", bind(&Gateway::OnRPCAddGroup, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("UpdateGroup", bind(&Gateway::OnRPCUpdateGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DELETE_GROUP", bind(&Gateway::OnRPCDelGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("ADD_DEVICE_TO_GROUP", bind(&Gateway::OnRPCAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("REMOVE_DEVICE_FROM_GROUP", bind(&Gateway::OnRPCDelDeviceFromGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("NEW_DEVICE", bind(&Gateway::OnRPCAddTuyaDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DelAllDevice", bind(&Gateway::OnRPCDelAllDevice, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("GetScanDevice", bind(&Gateway::OnRPCGetScanDevice, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("AddRule", bind(&Gateway::OnRPCAddRule, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("DeleteRule", bind(&Gateway::OnRPCDeleteRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DEVICE", bind(&Gateway::OnRPCControlDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("GROUP", bind(&Gateway::OnRPCControlGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DEVICE_UPDATE", bind(&Gateway::OnRPCUpdateAllTelemetry, this, placeholders::_1, placeholders::_2));
	// OnDeviceRPCCallbackRegister("SSHRemote", bind(&Gateway::OnRPCSSHRemote, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("DEVICE", bind(&Gateway::OnRPCControlDevice, this, placeholders::_1, placeholders::_2));

	CloudConnect();
	LocalConnect();
}

void Gateway::OnCloudConnect(bool isConnected, bool isReconnect)
{
	LOGI("OnCloudConnect: %d", isConnected);
	if (isConnected)
	{
		Util::LedInternet(true);
		OnlineHC(mac);
		if (!isReconnect)
		{
			for (const auto &[id, device] : deviceList)
			{
				device->PushAttributes();
			}
		}
	}
	else
	{
		Util::LedInternet(false);
	}
}

void Gateway::OnLocalConnect(bool isConnected, bool isReconnect)
{
	LOGI("OnLocalConnect: %d", isConnected);
}

int Gateway::UdpBroadcastThread()
{
	LOGI("Start UdpBroadcastThread");
	struct sockaddr_in s;
	memset(&s, 0, sizeof(struct sockaddr_in));
	s.sin_family = AF_INET;
	s.sin_port = htons(8181);
	string ip = Util::GetIP();
	LOGD("IP: %s", ip.c_str());
	if (ip.compare("10.10.10.1") == 0)
	{
		inet_pton(AF_INET, "10.10.10.255", &s.sin_addr);
	}
	else
	{
		s.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	}

	Json::Value hcBroadcastValue;
	Json::Value hcInfoValue;
	Json::Value appInfoValaue;
	Json::Value dataValue;
	hcBroadcastValue["CMD"] = "HC_BROADCAST";
	hcBroadcastValue["REQUEST_ID"] = Util::genRandRQI(16);
	hcBroadcastValue["TIME"] = Util::GetCurrentTimeStr();
	hcBroadcastValue["CONNECTION_TYPE"] = 0;
	hcInfoValue["TYPE"] = 2;
	hcInfoValue["DORMITORY_ID"] = dormitoryId;
	hcInfoValue["MAC"] = mac;
	hcInfoValue["VERSION"] = STR(VERSION);
	hcInfoValue["IP"] = Util::GetIP();
	hcBroadcastValue["FROM"] = hcInfoValue;
	appInfoValaue["TYPE"] = 0;
	hcBroadcastValue["TO"] = appInfoValaue;
	// hcBroadcastValue["DATA"] = dataValue;
	isUdpBroadcasting = true;
	bool ledInternet = Util::GetStatusLedInternet();
	for (int i = 0; i < 30; i++)
	{
		if (!isUdpBroadcasting)
			break;
		Util::LedInternet(false);
		usleep(500000);
		send(hcBroadcastValue.toString(), &s, sizeof(s));
		Util::LedInternet(true);
		usleep(500000);
	}
	Util::LedInternet(ledInternet);
	isUdpBroadcasting = false;
	free(udpBroadcastThread);
	udpBroadcastThread = NULL;
	return 0;
}

void Gateway::StartUdpBroadcast()
{
	if (!udpBroadcastThread)
	{
		udpBroadcastThread = new thread(bind(&Gateway::UdpBroadcastThread, this));
		udpBroadcastThread->detach();
	}
	else
	{
		LOGI("StartUdpBroadcast is still running...");
	}
}

int Gateway::OnUdpScanHc(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpScanHc");
	if (reqValue.isMember("DORMITORY_ID") && reqValue["DORMITORY_ID"].isString())
	{
		string dormitoryId = reqValue["DORMITORY_ID"].asString();
		if (this->dormitoryId != "" && this->dormitoryId != dormitoryId)
		{
			return -1;
		}
		respValue["CMD"] = "HC_RESPONSE";
		respValue["IP"] = Util::GetIP();
		respValue["HOSTNAME"] = "RD_HC_" + mac.substr(mac.size() - 4, 4);
		respValue["MAC"] = mac;
		respValue["TLS"] = false;
		respValue["MQTT_PORT"] = 1883;
		respValue["VERSION"] = STR(VERSION);
		return 0;
	}
	else
	{
		LOGW("OnUdpScanHc payload: %s error", reqValue.toString().c_str());
	}
	return -1;
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
                Util::ScanWifi(respValue,rqi);
				return 0;
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
	else {
		LOGW("OnUdpHcScanWifi payload: %s error", reqValue.toString().c_str());
	}
	
	return 0; // respValue as an array
}

int Gateway::GatewayConnectToCloudNotice()
{
	LOGD("OnGatewayConnectToCloudNotice");
	Json::Value respValue;
	Json::Value from;
	Json::Value to;
	Json::Value data;

	respValue["CMD"] = "HC_UPDATE";
	respValue["REQUEST_ID"] = Util::genRandRQI(16);
	respValue["TIME"] = Util::GetCurrentTimeStr();
	respValue["CONNECTION_TYPE"] = 3;

	from["TYPE"] = 2;
	from["DORMITORY_ID"] = dormitoryId;
	from["IP"] = Util::GetIP();
	from["MAC"] = Util::GetMacAddress();
	from["VERSION"] = STR(VERSION);
	respValue["FROM"] = from;

	to["TYPE"] = 1;
	to["ENVIRONMENT"] = "test";
	respValue["TO"] = to;

	// respValue["DATA"] = data;

	return CloudPublish("HC.CONTROL.RESPONSE", respValue.toString());
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
		fromRsp["MAC"] = Util::GetMacAddress();
		fromRsp["VERSION"] = STR(VERSION);

		if (from.isMember("TYPE") && from["TYPE"].isInt() && to.isMember("TYPE") && to["TYPE"].isInt())
		{
			if ((from["TYPE"].asInt() == 0) && to["TYPE"].asInt())
			{
				if (data.isMember("DORMITORY_ID") && data["DORMITORY_ID"].isString())
				{
					string dormitory = data["DORMITORY_ID"].asString();
					// TODO: compare or save dormitory
					fromRsp["IP"] = Util::GetIP();
					fromRsp["DORMITORY_ID"] = dormitory;
					respValue["FROM"] = fromRsp;
					if (Util::GetIP().compare("10.10.10.1") != 0)
					{
						dataRsp["STATUS"] = "SUCCESS";
						respValue["DATA"] = dataRsp;
						GatewayConnectToCloudNotice();
						return 0;
					}
					else
					{
						if (data.isMember("WIFI"))
						{
							Json::Value wifi = data["WIFI"];
							if (wifi.isMember("SSID") && wifi["SSID"].isString() &&
								wifi.isMember("PASSSWORD") && wifi["PASSSWORD"].isString() &&
								wifi.isMember("ENCRYPTION") && wifi["ENCRYPTION"].isString())
							{
								string ssid = wifi["SSID"].asString();
								string password = wifi["PASSSWORD"].asString();
								string encryption = wifi["ENCRYPTION"].asString();
								LOGD("ssid: %s, password: %s, encryption: %s", ssid.c_str(), password.c_str(), encryption.c_str());

								if (Util::ConnectToWifi(ssid, password, encryption) == 0)
								{
									dataRsp["STATUS"] = "SUCCESS";
									respValue["DATA"] = dataRsp;
									GatewayConnectToCloudNotice();
								}
								else
								{
									dataRsp["STATUS"] = "FAILED";
									respValue["DATA"] = dataRsp;
								}
								return 0;
							}
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
	}
	else
	{
		LOGW("OnUdpHcSetup payload: %s error", reqValue.toString().c_str());
	}
	return -1;
}

int Gateway::OnUdpHcConnectCloud(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnUdpHcConnectCloud");
	return 1;
}

int Gateway::OnRPCBleStartScan(Json::Value &reqValue, Json::Value &respValue)
{
	scanDeviceList.clear();
	bleProtocol->isAdding = true;
	bleProtocol->isProvisioning = true;
	if(bleProtocol->StartScan())
	{
		bleProtocol->StopScan();
	}
	return 1;
}

int Gateway::OnRPCBleStopScan(Json::Value &reqValue, Json::Value &respValue)
{
	bleProtocol->StopScan();
	bleProtocol->isAdding = false;
	bleProtocol->isProvisioning = false;
	respValue = reqValue;
	return 0;
}

int Gateway::OnRPCBleResetFactory(Json::Value &reqValue, Json::Value &respValue)
{
	bleProtocol->ResetFactory();
	bleProtocol->init();
	respValue["code"] = 0;
	return 0;
}

// int Gateway::OnRPCBleAddDevice(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("params") && reqValue["params"].isObject())
// 	{
// 		Json::Value dataValue = reqValue["params"];
// 		if ( // dataValue.isMember("id") && dataValue["id"].isString() &&
// 				dataValue.isMember("name") && dataValue["name"].isString() &&
// 				dataValue.isMember("mac") && dataValue["mac"].isString() &&
// 				dataValue.isMember("type") && dataValue["type"].isInt64())
// 		{
// 			string deviceId = ""; // dataValue["id"].asString();
// 			string name = dataValue["name"].asString();
// 			string mac = dataValue["mac"].asString();
// 			uint32_t type = dataValue["type"].asInt64();
// 			int rs = bleProtocol->AddDevice(deviceId, name, mac, type);
// 			respValue["code"] = rs;
// 			return 0;
// 		}
// 	}
// 	respValue["code"] = -1;
// 	return -1;
// }

int Gateway::OnRPCBleDelDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCBleDelDevice");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isArray())
	{
		Json::Value dataValue = reqValue["DATA"];
		for (Json::Value::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			string deviceId = dataValue[i].asString();
			LOGD("remove deviceId: %s", deviceId.c_str());
		}
	}
	else
	{
		LOGW("Format error");
	}
	return 0;
}

int Gateway::OnRPCAddRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddRule");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		Rule *rule = AddRule(dataValue, true, true);
		if (rule)
		{
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCDeleteRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDeleteRule");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("id") && dataValue["id"].isInt())
		{
			int ruleId = dataValue["id"].asInt();
			LOGI("Delete Rule id: %d", ruleId);
			delete ruleList[ruleId];
			ruleList.erase(ruleList.find(ruleId));
			database->RuleDel(ruleId);
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCAddGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddGroup");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
			dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray() &&
			dataValue.isMember("NAME") && dataValue["NAME"].isArray())
		{
			string groupId = dataValue["GROUP_ID"].asString();
			string groupName = dataValue["NAME"].asString();
			int temp_groupUnicastId = 0;
			for (auto &x : groupList)
			{
				if (x.first >= temp_groupUnicastId)
				{
					temp_groupUnicastId = x.first;
				}
			}
			Group *group = new Group(groupId, temp_groupUnicastId, groupName);
			if (group)
			{
				group = AddNewGroup(group, true, true);
				if (group)
				{
					Json::Value deviceList = dataValue["DEVICES"].isArray();
					for (int i = 0; i < (int)deviceList.size(); i++)
					{
						string devcieId = deviceList[i].asString();
						Device *device = getDeviceFromId(devcieId);
						if (group && device)
						{
							int tempDeviceAddr = device->GetAddr();
							if (group->AddDevice(device, tempDeviceAddr))
							{
								database->DeviceInGroupAdd(group, device, tempDeviceAddr);
								respValue["code"] = 0;
								return 0;
							}
						}
					}
					respValue["code"] = 0;
					return 0;
				}
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCUpdateGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCUpdateGroup");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("id") && dataValue["id"].isInt() &&
			dataValue.isMember("name") && dataValue["name"].isString())
		{
			int groupId = dataValue["id"].asInt();
			string name = dataValue["name"].asString();
			Group *group = getGroup(groupId);
			if (group)
			{
				group->SetName(name);
				database->GroupUpdate(group);
				respValue["code"] = 0;
				return 0;
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCDelGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDelGroup");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isInt())
		{
			string groupId = dataValue["GROUP_ID"].asString();
			LOGI("Delete Rule id: %s", groupId.c_str());
			Group *group = getGroupFromId(groupId);
			int temp_groupUnicastId = group->GetId();
			delete groupList[temp_groupUnicastId];
			groupList.erase(groupList.find(temp_groupUnicastId));
			for (size_t i = 0; i < group->deviceList.size(); i++)
			{
				group->DelDevice(group->deviceList[i]->device, group->deviceList[i]->epId);
			}
			database->GroupDel(temp_groupUnicastId);
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddDeviceToGroup");
	try
	{
		if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
		{
			Json::Value dataValue = reqValue["DATA"];
			if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
				dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
			{
				string groupId = dataValue["GROUP_ID"].asString();
				Json::Value deviceList = dataValue["DEVICES"].isArray();
				Group *group = getGroupFromId(groupId);
				for (int i = 0; i < (int)deviceList.size(); i++)
				{
					string devcieId = deviceList[i].asString();
					Device *device = getDeviceFromId(devcieId);
					if (group && device)
					{
						int tempDeviceAddr = device->GetAddr();
						if (group->AddDevice(device, tempDeviceAddr))
						{
							database->DeviceInGroupAdd(group, device, tempDeviceAddr);
							respValue["code"] = 0;
							return 0;
						}
					}
				}
			}
		}
		respValue["code"] = -1;
		return -1;
	}
	catch (const char *msg)
	{
		LOGE("OnRPCAddDeviceToGroup fail");
		return -1;
	}
}

/**
 * @brief delete device from group
 *
 * @param [in] reqValue json input
 * @param [out] respValue json output
 * @return int -1 - error, 0 - success
 */
int Gateway::OnRPCDelDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDelDeviceFromGroup");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
			dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
		{
			int groupId = dataValue["GROUP_ID"].asInt();
			Group *group = getGroup(groupId);
			Json::Value deviceList = dataValue["DEVICES"];
			for (int i = 0; i < (int)deviceList.size(); i++)
			{
				string deviceId = deviceList[i].asString();
				Device *device = getDeviceFromId(deviceId);
				if (group && device)
				{
					int tempDeviceAddr = device->GetAddr();
					if (group->AddDevice(device, tempDeviceAddr))
					{
						database->DeviceInGroupDel(group, device, tempDeviceAddr);
						respValue["code"] = 0;
						return 0;
					}
				}
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

/**
 * @brief
 *
 * @param reqValue
 * @param respValue
 * @return int
 */
int Gateway::OnRPCAddDevice(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("id") && dataValue["id"].isString() &&
			dataValue.isMember("name") && dataValue["name"].isString() &&
			dataValue.isMember("mac") && dataValue["mac"].isString() &&
			dataValue.isMember("addr") && dataValue["addr"].isInt() &&
			dataValue.isMember("type") && dataValue["type"].isInt() &&
			dataValue.isMember("devicekey") && dataValue["devicekey"].isString() &&
			dataValue.isMember("version") && dataValue["version"].isInt())
		{
			string deviceId = dataValue["id"].asString();
			string name = dataValue["name"].asString();
			string mac = dataValue["mac"].asString();
			uint32_t addr = dataValue["addr"].asInt();
			uint32_t type = dataValue["type"].asInt();
			string devicekey = dataValue["devicekey"].asString();
			uint16_t version = dataValue["version"].asInt();
			Device *device = AddNewDevice(deviceId, name, mac, devicekey,addr, type, version, true, true);
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCAddTuyaDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddTuyaDevice");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() &&
			dataValue.isMember("PROPERTIES") && dataValue["PROPERTIES"].isArray())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			Json::Value properties = dataValue["PROPERTIES"];
			LOGI("New Tuya Device: %s", deviceId.c_str());
			// Device *device = getDeviceFromId(deviceId);
			// if (device)
			// {
			// 	for (Json::Value::ArrayIndex i = 0; i < properties.size(); i++)
			// 	{
			// 		Json::Value property = properties[i];
			// 		if (property.isObject() &&
			// 				property.isMember("ID") && property["ID"].isInt() &&
			// 				property.isMember("CMD") && property["CMD"].isString())
			// 		{
			// 			int id = property["ID"].asInt();
			// 			string cmd = property["CMD"].asString();
			// 			device->Do(id, value);
			// 		}
			// 	}
			// }
			// else
			// {
			// 	LOGW("Device not found");
			// }
		}
		else
		{
			LOGW("Format error");
		}
	}
	else
	{
		LOGW("Format error");
	}
	respValue = reqValue;
	return 0;
}

//
int Gateway::OnRPCDelAllDevice(Json::Value &reqValue, Json::Value &respValue)
{
	database->DeviceDelAll();
	deviceList.clear();
	bleProtocol->ResetFactory();
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCGetScanDevice(Json::Value &reqValue, Json::Value &respValue)
{
	Json::Value scanDeviceValues;
	for (auto &scanDevice : scanDeviceList)
	{
		Json::Value scanDeviceValue;
		scanDeviceValue["name"] = scanDevice->GetName();
		scanDeviceValue["mac"] = scanDevice->GetMac();
		scanDeviceValue["type"] = scanDevice->GetType();
		scanDeviceValue["version"] = scanDevice->GetVersion();
		scanDeviceValue["rssi"] = scanDevice->GetRSSI();
		scanDeviceValues.append(scanDeviceValue);
		// delete scanDevice;
	}
	respValue["devices"] = scanDeviceValues;
	respValue["count"] = scanDeviceList.size();
	respValue["code"] = 0;
	// scanDeviceList.clear();
	Json::Value jsonValue;
	jsonValue["HaveNewDevice"] = false;
	PublishToDeviceTelemetry(jsonValue);
	return 0;
}

int Gateway::OnRPCControlDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCControlDevice");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() &&
			dataValue.isMember("PROPERTIES") && dataValue["PROPERTIES"].isArray())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			Json::Value properties = dataValue["PROPERTIES"];
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (properties.isArray())
					device->Do(properties);
			}
			else
			{
				LOGW("Device not found");
			}
		}
		else
		{
			LOGW("Format error");
		}
	}
	else
	{
		LOGW("Format error");
	}
	respValue = reqValue;
	return 1;
}

int Gateway::OnRPCControlGroup(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
			dataValue.isMember("PROPERTIES") && dataValue["PROPERTIES"].isArray())
		{
			string groupId = dataValue["GROUP_ID"].asString();
			Json::Value properties = dataValue["PROPERTIES"];
			Group *group = getGroupFromId(groupId);
			if (group)
			{
				for (Json::Value::ArrayIndex i = 0; i < properties.size(); i++)
				{
					Json::Value property = properties[i];
					if (property.isObject() &&
						property.isMember("ID") && property["ID"].isInt() &&
						property.isMember("VALUE") && property["VALUE"].isInt())
					{
						int id = property["ID"].asInt();
						int value = property["VALUE"].asInt();
						group->Do(id, value);
					}
				}
			}
			else
			{
				LOGW("Group not found");
			}
		}
		else
		{
			LOGW("Format error");
		}
	}
	else
	{
		LOGW("Format error");
	}
	return 0;
}

int Gateway::OnRPCUpdateAllTelemetry(Json::Value &reqValue, Json::Value &respValue)
{
	Json::Value dataValue;
	Json::Value deviceValue;
	Json::Value propertiesValue;
	for (const auto &[id, device] : deviceList)
	{
		propertiesValue = Json::Value::null;
		deviceValue = Json::Value::null;
		device->BuildTelemetryValue(propertiesValue);
		deviceValue["DEVICE_ID"] = device->GetId();
		deviceValue["PROPERTIES"] = propertiesValue;
		dataValue.append(deviceValue);
	}
	respValue["CMD"] = "DEVICE_UPDATE";
	respValue["DATA"] = dataValue;
	return 0;
}

int Gateway::OnRPCSSHRemote(Json::Value &reqValue, Json::Value &respValue)
{
	int rs = 0;
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("type") && dataValue["type"].isString() &&
			dataValue.isMember("key") && dataValue["key"].isString() &&
			dataValue.isMember("user") && dataValue["user"].isString() &&
			dataValue.isMember("host") && dataValue["host"].isString() &&
			dataValue.isMember("serverPort") && dataValue["serverPort"].isInt() &&
			dataValue.isMember("forwardPort") && dataValue["forwardPort"].isInt())
		{
			string key = "";
			string type = dataValue["type"].asString();
			string user = dataValue["user"].asString();
			string host = dataValue["host"].asString();
			uint32_t serverPort = dataValue["serverPort"].asInt();
			uint32_t forwardPort = dataValue["forwardPort"].asInt();
			uint32_t localPort = 22;
			if (dataValue.isMember("localPort") && dataValue["localPort"].isInt())
			{
				localPort = dataValue["localPort"].asInt();
			}
			if (type == "base64")
			{
				string keyBase64 = dataValue["key"].asString();
				string encode = macaron::Base64::Decode(keyBase64, key);
				if (encode != "")
				{
					rs = 1;
				}
			}
			else
			{
				key = dataValue["key"].asString();
			}

			if (rs == 0)
			{
				// save key file
				system("rm /key.txt");
				system("rm /output.txt");
				ofstream keyFile("/key.txt");
				keyFile << key;
				keyFile.close();

				system("chmod 600 /key.txt");
				system("killall ssh");
				string cmd = "ssh -i /key.txt -o StrictHostKeyChecking=no -f -N -T -R" + to_string(forwardPort) + ":localhost:" + to_string(localPort) + " " + user + "@" + host + " -p " + to_string(serverPort);
				cmd += " >> /output.txt 2>&1";
				LOGI("cmd: %s", cmd.c_str());
				system(cmd.c_str());
				sleep(2);
				bool err = false;
				FILE *fp = fopen("/output.txt", "r");
				char path[512] = {0};
				if (fp)
				{
					while (fgets(path, sizeof(path), fp) != NULL)
					{
						if (strlen(path) > 1)
						{
							LOGW("SSH err: %s", path);
							err = true;
							break;
						}
					}
					fclose(fp);
				}
				if (err)
				{
					respValue["msg"] = string(path);
					respValue["code"] = 1;
				}
				else
				{
					respValue["code"] = 0;
				}
				return 0;
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

void Gateway::AddDeviceToScanList(Device *scanDevice)
{
	Json::Value jsonValue;
	Json::Value dataValue;
	if (!scanDevice)
	{
		LOGW("scanDevice null");
		return;
	}
	dataValue["DEVICE_ID"] = scanDevice->GetId();
	dataValue["DEVICE_UNICAST_ID"] = scanDevice->GetAddr();
	dataValue["DEVICE_TYPE_ID"] = scanDevice->GetType();
	dataValue["MAC_ADDRESS"] = scanDevice->GetMac();
	dataValue["FIRMWARE_VERSION"] = scanDevice->GetVersionStr();
	dataValue["DEVICE_KEY"] = scanDevice->GetDeviceId();
	dataValue["NET_KEY"] = gateway->getBleNetkey();
	dataValue["APP_KEY"] = gateway->getBleAppKey();
	jsonValue["CMD"] = "NEW_DEVICE";
	jsonValue["DATA"] = dataValue;
	PublishToDeviceTelemetry(jsonValue);
}

Group *Gateway::getGroup(int id)
{
	if (groupList.find(id) != groupList.end())
	{
		return groupList[id];
	}
	return NULL;
}

Group *Gateway::getGroupFromId(string groupId)
{
	for (const auto &[id, group] : groupList)
	{
		if (group->GetName() == groupId)
			return group;
	}
	return NULL;
}

Device *Gateway::getDevice(string mac)
{
	if (deviceList.find(mac) != deviceList.end())
	{
		return deviceList[mac];
	}
	return NULL;
}

Device *Gateway::getDeviceFromId(string deviceId)
{
	for (const auto &[id, device] : deviceList)
	{
		if (device->GetId() == deviceId)
			return device;
	}
	return NULL;
}

DeviceBle *Gateway::getDeviceBleFromAddr(uint32_t addr)
{
	for (const auto &[id, device] : deviceList)
	{
		if (device->CheckAddr(addr) && device->GetProtocol() == BLE_DEVICE)
		{
			DeviceBle *deviceBle = dynamic_cast<DeviceBle *>(device);
			if (deviceBle)
				return deviceBle;
		}
	}
	return NULL;
}

#ifdef CONFIG_ENABLE_ZIGBEE
DeviceZigbee *Gateway::getDeviceZigbeeFromAddr(uint32_t addr)
{
	for (const auto &[id, device] : deviceList)
	{
		if (device->CheckAddr(addr) && device->GetProtocol() >= ZIGBEE_DEVICE)
		{
			DeviceZigbee *deviceZigbee = dynamic_cast<DeviceZigbee *>(device);
			if (deviceZigbee)
				return deviceZigbee;
		}
	}
	return NULL;
}
#endif

Device *Gateway::AddNewDevice(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version, bool addGateway, bool addDatabase)
{
	LOGI("Add new device id: %s, name: %s, mac: %s, addr: 0x%04X, type: 0x%04X, verion: %d", id.c_str(), name.c_str(), mac.c_str(), addr, type, version);
	Device *device = NULL;
	if (type == BLE_DOWNLIGHT_SMT)
	{
		device = new DeviceBleDownLightSmt(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_DOWNLIGHT_COB_TRANG_TRI)
	{
		device = new DeviceBleDownLightCobTrangTri(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_DOWNLIGHT_COB_GOC_RONG) 
	{
		device = new DeviceBleDownLightCobGocRong(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_DOWNLIGHT_COB_GOC_HEP)
	{
		device = new DeviceBleDownLightCobGocHep(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_DOWNLIGHT_RGBCW)
	{
		device = new DeviceBleDownLightRgbCw(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_PANEL_TRON)
	{
		device = new DeviceBlePanelTron(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_PANEL_VUONG)
	{
		device = new DeviceBlePanelVuong(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_OP_TRAN)
	{
		device = new DeviceBleOpTran(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_OP_TUONG)
	{
		device = new DeviceBleOpTuong(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_CHIEU_TRANH)
	{
		device = new DeviceBleChieuTranh(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_TRACKLIGHT)
	{
		device = new DeviceBleTrackLight(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_THA_TRAN)
	{
		device = new DeviceBleThaTran(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_CHIEU_GUONG)
	{
		device = new DeviceBleChieuGuong(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_DAY_LINEAR)
	{
		device = new DeviceBleLedDayLinear(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_TUBE_M16)
	{
		device = new DeviceBleTubeM16(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_DEN_BAN)
	{
		device = new DeviceBleDenBan(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_FLOOD)
	{
		device = new DeviceBleFlood(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_DAY_RGBCW)
	{
		device = new DeviceBleLedDayRgbCw(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_BULB)
	{
		device = new DeviceBleBulb(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_OP_TRAN_LOA)
	{
		device = new DeviceBleOpTranLoa(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_LED_DAY_RGB)
	{
		device = new DeviceBleLedDayRgb(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_SWITCH_4)
	{
		device = new DeviceBleSwitch4(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_DC_SCENE_CONTACT)
	{
		device = new DeviceBleDCSceneContact(id, name, mac, device_id, addr, version);
	}
	else if (type == BLE_TEMP_HUM_SENSOR)
	{
		device = new DeviceBleTempHumSensor(id, name, mac, device_id, addr, version);
	}

#ifdef CONFIG_ENABLE_ZIGBEE
	if (type == ZIGBEE_LUMI_PLUG)
	{
		device = new DeviceZigbeeOnoff(id, name, mac, addr);
	}
	else if (type == ZIGBEE_TELINK_TLSR82xx)
	{
		device = new DeviceZigbeeTelinkOnoff(id, name, mac, addr);
	}
#endif

	if (device)
	{
		if (addGateway)
			deviceList[mac] = device;
		if (addDatabase)
			database->DeviceAdd(device);
		// if (connected)
		// 	device->PushAttributes();
	}
	else
	{
		LOGW("Add new device not support type: 0x%04X", type);
	}
	return device;
}

Group *Gateway::AddNewGroup(Group *group, bool addGateway, bool addDatabase)
{
	if (group)
	{
		if (addDatabase)
		{
			int rs = database->GroupAdd(group);
			if (rs)
			{
				LOGW("rs: %d", rs);
				return NULL;
			}
		}
		if (addGateway)
			groupList[group->GetId()] = group;
	}
	return group;
}

Rule *Gateway::AddRule(Json::Value &ruleValue, bool addGateway, bool addDatabase)
{
	// TODO: Check Rule id exist
	if (ruleValue.isMember("id") && ruleValue["id"].isInt() &&
		ruleValue.isMember("repeat") && ruleValue["repeat"].isInt() &&
		ruleValue.isMember("fullDay") && ruleValue["fullDay"].isBool() &&
		ruleValue.isMember("type") && ruleValue["type"].isString() &&
		ruleValue.isMember("input") && ruleValue["input"].isObject() &&
		ruleValue.isMember("output") && ruleValue["output"].isObject())
	{
		int id = ruleValue["id"].asInt();
		int repeat = ruleValue["repeat"].asInt();
		bool fullDay = ruleValue["fullDay"].asBool();
		string type = ruleValue["type"].asString();
		Rule *rule = NULL;
		if (!fullDay && ruleValue.isMember("startTime") && ruleValue["startTime"].isString() &&
			ruleValue.isMember("endTime") && ruleValue["endTime"].isString())
		{
			string startTime = ruleValue["startTime"].asString();
			string endTime = ruleValue["endTime"].asString();
			rule = new Rule(id, type, repeat, Util::ConvertStrTimeToInt(startTime), Util::ConvertStrTimeToInt(endTime));
		}
		else
		{
			rule = new Rule(id, type, repeat);
		}
		if (!rule)
		{
			LOGE("New rule error");
			return NULL;
		}
		Json::Value inputValue = ruleValue["input"];
		if (inputValue.isMember("timer") && inputValue["timer"].isObject())
		{
			Json::Value timerValue = inputValue["timer"];
			if (timerValue.isMember("repeat") && timerValue["repeat"].isInt() &&
				timerValue.isMember("time") && timerValue["time"].isString())
			{
				int repeat = timerValue["repeat"].asInt();
				string timerStr = timerValue["time"].asString();
				LOGI("Have Timer: %s", timerStr.c_str());
				int timer = Util::ConvertStrTimeToInt(timerStr);
				if (timer > 0)
				{
					RuleInputTimer *ruleInputTimer = new RuleInputTimer(rule, timer, repeat);
					rule->AddRuleInput(ruleInputTimer);
				}
			}
		}
		if (inputValue.isMember("device") && inputValue["device"].isArray())
		{
			Json::Value deviceRuleInputList = inputValue["device"];
			for (Json::Value::ArrayIndex i = 0; i < deviceRuleInputList.size(); i++)
			{
				Json::Value deviceRuleInputValue = deviceRuleInputList[i];
				if (deviceRuleInputValue.isObject())
				{
					if (deviceRuleInputValue.isMember("mac") && deviceRuleInputValue["mac"].isString() &&
						deviceRuleInputValue.isMember("data") && deviceRuleInputValue["data"].isObject())
					{
						string mac = deviceRuleInputValue["mac"].asString();
						Json::Value dataValue = deviceRuleInputValue["data"];
						Device *device = getDevice(mac);
						if (device)
						{
							RuleInputDevice *ruleInputDevice = new RuleInputDevice(rule, device, dataValue);
							rule->AddRuleInput(ruleInputDevice);
						}
					}
				}
			}
		}

		Json::Value outputValue = ruleValue["output"];
		if (outputValue.isMember("device") && outputValue["device"].isArray())
		{
			Json::Value deviceRuleOutputList = outputValue["device"];
			for (Json::Value::ArrayIndex i = 0; i < deviceRuleOutputList.size(); i++)
			{
				Json::Value deviceRuleOutputValue = deviceRuleOutputList[i];
				if (deviceRuleOutputValue.isObject())
				{
					if (deviceRuleOutputValue.isMember("mac") && deviceRuleOutputValue["mac"].isString() &&
						deviceRuleOutputValue.isMember("data") && deviceRuleOutputValue["data"].isObject())
					{
						Json::Value dataValue = deviceRuleOutputValue["data"];
						string mac = deviceRuleOutputValue["mac"].asString();
						Device *device = getDevice(mac);
						if (device)
						{
							RuleOutputDevice *ruleOutputDevice = new RuleOutputDevice(device, dataValue);
							rule->AddRuleOutput(ruleOutputDevice);
						}
					}
				}
			}
		}
		if (outputValue.isMember("group") && outputValue["group"].isArray())
		{
			Json::Value groupRuleOutputList = outputValue["group"];
			for (Json::Value::ArrayIndex i = 0; i < groupRuleOutputList.size(); i++)
			{
				Json::Value groupRuleOutputValue = groupRuleOutputList[i];
				if (groupRuleOutputValue.isObject())
				{
					if (groupRuleOutputValue.isMember("id") && groupRuleOutputValue["id"].isInt() &&
						groupRuleOutputValue.isMember("data") && groupRuleOutputValue["data"].isObject())
					{
						int id = groupRuleOutputValue["id"].asInt();
						Json::Value dataValue = groupRuleOutputValue["data"];
						Group *group = getGroup(id);
						if (group)
						{
							RuleOutputGroup *ruleOutputGroup = new RuleOutputGroup(group, dataValue);
							rule->AddRuleOutput(ruleOutputGroup);
						}
					}
				}
			}
		}
		LOGI("Add Rule %d", rule->GetId());
		if (addGateway)
			ruleList[rule->GetId()] = rule;
		if (addDatabase)
		{
			string ruleStr = ruleValue.toString();
			ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
			database->RuleAdd(rule->GetId(), ruleStr);
		}
		rule->Check();
		return rule;
	}
	else
	{
		LOGW("Rule format error");
	}
	return NULL;
}

string Gateway::getBleNetkey()
{
	return ble_netkey;
}
string Gateway::getBleAppKey()
{
	return ble_appkey;
}
string Gateway::getBleDeviceKey()
{
	return ble_devicekey;
}
string Gateway::getDormitory()
{
	return dormitoryId;
}
string Gateway::getId()
{
	return id;
}
string Gateway::getVersion()
{
	return version;
}
string Gateway::getName()
{

}

void Gateway::setBleNetkey(string netkey)
{
	this->ble_netkey = netkey;
}
void Gateway::setBleAppkey(string appkey)
{
	this->ble_appkey = appkey;
}
void Gateway::setBleDevicekey(string devicekey)
{
	this->ble_devicekey = devicekey;
}
void Gateway::setDormitory(string dormitory)
{
	this->dormitoryId = dormitory;
}
void Gateway::setId(string id)
{
	this->id = id;
}
void Gateway::setVersion(string version)
{
	this->version = version;
}
void Gateway::setName(string name)
{

}