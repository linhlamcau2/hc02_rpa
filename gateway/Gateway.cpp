#include "Gateway.h"
#include "Log.h"
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
#include "Wifi.h"
#include "Ota.h"
#include "Base64.h"
#ifndef ESP_PLATFORM
#include "Config.h"
#endif
#include "Http.h"

#include "RuleInputTimer.h"
#include "RuleOutputGroup.h"
#include "RuleOutputDevice.h"

#include "BleDefine.h"
#include "BleProtocol.h"
#include "DeviceBleLightOnoffCctDim.h"
#include "DeviceBleLightOnoffHslModeRGB.h"
#include "DeviceBleLightOnoffCctDimHslModeRGB.h"
#include "DeviceBleSwitchTouchRgb4.h"
#include "DeviceBleSwitchScene6DC.h"
#include "DeviceBleSensorTempHum.h"
#include "DeviceBleSensorPm.h"
#include "DeviceBlePirLightSensorDC.h"
#include "DeviceBleSmokeSensor.h"
#include "DeviceBleDoorSensor.h"
#include "DeviceBleScreenTouch.h"

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#include "DeviceZigbeeOnoff.h"
#include "DeviceZigbeeTelinkOnoff.h"
#endif

Gateway *gateway = NULL;

Gateway::Gateway(string mac, string server_address, int server_port, string token, string username, string password, int keepalive, string localIp, int localPort, string localUsername, string localPassword, int localKeepalive)
		: CloudProtocol(mac, server_address, server_port, token, username, password, keepalive),
			LocalProtocol(mac, localIp, localPort, mac, localUsername, localPassword, localKeepalive),
			Udp(8181)
{
	this->mac = mac;
	this->id = "";
	this->dormitoryId = "";
	this->refresh_token = "";
	this->ble_unicast = 0;
	this->ble_appkey = "";
	this->ble_appkey = "";
	this->ble_devicekey = "";
	udpBroadcastThread = NULL;
}

void Gateway::init()
{
	CloudProtocol::init();
	LocalProtocol::init();
	Udp::init();

	initUdpMessage();
	initMqttMessage();
	initMqttMessageV2();

	LOGI("DeviceRead");
	database->GatewayRead();
	database->DeviceRead();
	database->DeviceAttributeRead();
	database->GroupRead();
	database->DeviceInGroupRead();
	database->RuleRead();
	database->SceneBleRead();
	database->RoomRead();
	database->DataRoomRead();
	database->DeviceInRoomRead();
	if (gateway->getId().compare("") == 0)
	{
		id = mac;
		gateway->setId(id);
		database->GatewayUpdateId(gateway, id);
		database->GatewayRead();
	}

	CloudConnect();
	LocalConnect();

	thread checkOnlineThread(bind(&Gateway::CheckOnlineThread, this));
	checkOnlineThread.detach();
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

void Gateway::ResetFactory()
{
	LOGI("ResetFactory");
	deviceList.clear();
	groupList.clear();
	ruleList.clear();
	sceneBleList.clear();
	scanDeviceList.clear();

	database->DeviceDelAll();
	database->GatewayDelAll();
	database->GatewayUpdateId(gateway, gateway->getId());
	gateway->setBleAppkey("");
	database->DeviceAttributeDelAll();
	database->GroupDelAll();
	database->DeviceInGroupDelAll();
	bleProtocol->ResetDelAll();
	bleProtocol->ResetFactory();
}

int Gateway::CheckOnlineThread()
{
	LOGI("Start CheckOnlineThread");
	time_t currentTime = 0;
	uint32_t allTimeCheck = 0; // time total in a loop check
	bool deviceStateChange = false;
	Json::Value onlineValue;
	Json::Value datasValue;
	Json::Value dataValue;
	Json::Value propertysValue;
	Json::Value propertyValue;
	propertyValue["ID"] = BLE_ATTRIBUTE_ONLINE_OFFLINE;
	propertyValue["VALUE"] = 1;
	propertysValue.append(propertyValue);
	dataValue["DEVICE_ID"] = "";
	dataValue["PROPERTIES"] = propertysValue;
	datasValue.append(dataValue);
	onlineValue["CMD"] = "DEVICE";
	onlineValue["DATA"] = datasValue;

	while (!bleProtocol)
	{
		sleep(1);
	}

	while (1)
	{
		currentTime = time(NULL);
		allTimeCheck = deviceList.size() * 4;
		for (const auto &[id, device] : deviceList)
		{
			deviceStateChange = false;
			if (device->lastOnlineState) // online
			{
				// neu thiet bi ho tro ban tin check trang thai online/offline
				if (device->isNeedCheckOnline())
				{
					// thoi gian lan cuoi cung nhan ban tin hoac lan cuoi cung check qua 1 chu ky
					if ((device->lastTimeActive + allTimeCheck) <= currentTime && (device->lastTimeCheck + allTimeCheck) <= currentTime)
					{
						bleProtocol->SendOnlineCheck(device->GetAddr());
						device->lastTimeCheck = currentTime;
					}
					// 2 chu ky khong co ban tin phan hoi thi bao offline
					if ((device->lastTimeActive + allTimeCheck * 2) < currentTime)
					{
						LOGI("Device 0x%04X offline", device->GetAddr());
						device->lastOnlineState = false;
						deviceStateChange = true;
					}
				}
				// neu thiet bi khong ho tro ban tin check trang thai online/offline
				else
				{
					// 1 ngay khong co ban tin moi thi bao offline
					if ((device->lastTimeActive + 60 * 60 * 24) < currentTime)
					{
						LOGI("Device 0x%04X offline", device->GetAddr());
						device->lastOnlineState = false;
						deviceStateChange = true;
					}
				}
			}
			else
			{
				if (device->isNeedCheckOnline())
				{
					// thoi gian check qua 1 chu ky thi check lai
					if ((device->lastTimeCheck + allTimeCheck) <= currentTime)
					{
						bleProtocol->SendOnlineCheck(device->GetAddr());
						device->lastTimeCheck = currentTime;
					}
					// neu co ban tin moi trong vong 2 chu ky check thi bao online
					if ((device->lastTimeActive + allTimeCheck * 2) >= currentTime)
					{
						LOGI("Device 0x%04X online", device->GetAddr());
						device->lastOnlineState = true;
						deviceStateChange = true;
					}
				}
				else
				{
					// trong ngay co ban tin thi online
					if ((device->lastTimeActive + 60 * 60 * 24) >= currentTime)
					{
						LOGI("Device 0x%04X online", device->GetAddr());
						device->lastOnlineState = true;
						deviceStateChange = true;
					}
				}
			}
			// send device state to server
			if (deviceStateChange)
			{
				onlineValue["DATA"][0]["DEVICE_ID"] = device->GetId();
				onlineValue["DATA"][0]["PROPERTIES"][0]["VALUE"] = (int)device->lastOnlineState;
				PublishToLocalMessage(onlineValue);
				PublishToGatewayTelemetry(onlineValue);
			}
		}
		sleep(1);
	}
	return 0;
}

int Gateway::UdpBroadcastThread()
{
	LOGI("Start UdpBroadcastThread");
	struct sockaddr_in s;
	memset(&s, 0, sizeof(struct sockaddr_in));
	s.sin_family = AF_INET;
	s.sin_port = htons(8181);
	string ip = Wifi::GetIP();
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
	hcInfoValue["IP"] = Wifi::GetIP();
	hcBroadcastValue["FROM"] = hcInfoValue;
	appInfoValaue["TYPE"] = 0;
	hcBroadcastValue["TO"] = appInfoValaue;
	if (Wifi::GetIP().compare("10.10.10.1") == 0)
	{
		dataValue["CONNECTION_MODE"] = "Wifi";
	}
	else
	{
		dataValue["CONNECTION_MODE"] = "Lan";
	}
	hcBroadcastValue["DATA"] = dataValue;
	isUdpBroadcasting = true;
	bool ledInternet = Util::GetStatusLedInternet();
	for (int i = 0; i < 120; i++)
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

void Gateway::StopUdpBroadcast()
{
	isUdpBroadcasting = false;
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
	from["IP"] = Wifi::GetIP();
	from["MAC"] = mac;
	from["VERSION"] = STR(VERSION);
	respValue["FROM"] = from;

	to["TYPE"] = 1;
	to["ENVIRONMENT"] = "test";
	respValue["TO"] = to;

	// respValue["DATA"] = data;

	return CloudPublish("HC.CONTROL.RESPONSE", respValue.toString());
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
	dataValue["DEVICE_UNICAST_ID"] = (int)scanDevice->GetAddr();
	dataValue["DEVICE_TYPE_ID"] = (int)scanDevice->GetType();
	dataValue["MAC_ADDRESS"] = scanDevice->GetMac();
	dataValue["FIRMWARE_VERSION"] = scanDevice->GetVersionStr();
	dataValue["DEVICE_KEY"] = scanDevice->GetDeviceId();
	dataValue["NET_KEY"] = gateway->getBleNetkey();
	dataValue["APP_KEY"] = gateway->getBleAppKey();
	jsonValue["CMD"] = "NEW_DEVICE";
	jsonValue["DATA"] = dataValue;
#ifdef CONFIG_USE_OLD_APP
	PublishToLocalMessage(jsonValue);
#else
	PublishToDeviceTelemetry(jsonValue);
#endif
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
		if (group->GetUUId() == groupId)
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

Rule *Gateway::getRuleById(string eventId)
{
	for (const auto &[id, rule] : ruleList)
	{
		if (rule->GetId() == eventId)
		{
			return rule;
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

SceneBle *Gateway::getSceneBleFromId(string sceneBleUUId)
{
	for (const auto &[id, scene] : sceneBleList)
	{
		if (scene->GetUUId() == sceneBleUUId)
			return scene;
	}
	return NULL;
}

Room *Gateway::getRoomFromId(string roomUUId)
{
	for (auto &[id, room] : roomList)
	{
		if (id == roomUUId)
			return room;
	}
	return NULL;
}

Device *Gateway::AddNewDevice(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version, bool addGateway, bool addDatabase)
{
	LOGI("Add new device id: %s, name: %s, mac: %s, addr: 0x%04X, type: 0x%04X, verion: %d", id.c_str(), name.c_str(), mac.c_str(), addr, type, version);
	Device *device = NULL;
	switch (type)
	{
	case BLE_LED_CHIEU_TRANH:
	case BLE_LED_CHIEU_GUONG:
	case BLE_DEN_BAN:
	case BLE_DOWNLIGHT_SMT:
	case BLE_DOWNLIGHT_COB_GOC_HEP:
	case BLE_DOWNLIGHT_COB_GOC_RONG:
	case BLE_DOWNLIGHT_COB_TRANG_TRI:
	case BLE_LED_FLOOD:
	case BLE_LED_DAY_LINEAR:
	case BLE_LED_OP_TRAN:
	case BLE_LED_OP_TUONG:
	case BLE_LED_OP_TRAN_LOA:
	case BLE_PANEL_TRON:
	case BLE_PANEL_VUONG:
	case BLE_TRACKLIGHT:
	case BLE_LED_THA_TRAN:
	case BLE_LED_TUBE_M16:
		device = new DeviceBleLightOnoffCctDim(id, name, mac, device_id, addr, type, version);
		break;
	case BLE_DOWNLIGHT_RGBCW:
	case BLE_LED_DAY_RGBCW:
	case BLE_LED_BULB:
		device = new DeviceBleLightOnoffCctDimHslModeRGB(id, name, mac, device_id, addr, type, version);
		break;
	case BLE_LED_DAY_RGB:
		device = new DeviceBleLightOnoffHslModeRGB(id, name, mac, device_id, addr, type, version);
		break;
	case BLE_SWITCH_RGB_4:
		device = new DeviceBleSwitchTouchRgb4(id, name, mac, device_id, addr, version);
		break;
	case BLE_DC_SCENE_CONTACT:
		device = new DeviceBleSwitchScene6DC(id, name, mac, device_id, addr, version);
		break;
	case BLE_TEMP_HUM_SENSOR:
		device = new DeviceBleSensorTempHum(id, name, mac, device_id, addr, version);
		break;
	case BLE_PM_SENSOR:
		device = new DeviceBleSensorPm(id, name, mac, device_id, addr, version);
		break;
	case BLE_PIR_LIGHT_SENSOR_DC:
		device = new DeviceBlePirLightSensorDC(id, name, mac, device_id, addr, version);
		break;
	case BLE_SMOKE_SENSOR:
		device = new DeviceBleSmokeSensor(id, name, mac, device_id, addr, version);
		break;
	case BLE_DOOR_SENSOR:
		device = new DeviceBleDoorSensor(id, name, mac, device_id, addr, version);
		break;
	case BLE_AC_SCENE_SCREEN_TOUCH:
		device = new DeviceBleScreenTouch(id, name, mac, device_id, addr, version);
		break;
	default:
		LOGW("Add new device not support type: 0x%04X", type);
		break;
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
		device->lastTimeActive = time(NULL);
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
	if (ruleValue.isMember("EVENT_TRIGGER_ID") && ruleValue["EVENT_TRIGGER_ID"].isString() &&
			ruleValue.isMember("LOGICAL_OPERATOR_ID") && ruleValue["LOGICAL_OPERATOR_ID"].isInt() &&
			ruleValue.isMember("STATUS") && ruleValue["STATUS"].isInt() &&
			ruleValue.isMember("EACH_DAY") && ruleValue["EACH_DAY"].isArray())
	{
		int status = ruleValue["STATUS"].asInt();
		string id = ruleValue["EVENT_TRIGGER_ID"].asString();
		Json::Value repeatDays = ruleValue["EACH_DAY"];
		int mon = 0, tue = 0, wed = 0, thu = 0, fri = 0, sat = 0, sun = 0;
		int repeat;
		if (repeatDays.size() > 0)
		{
			for (Json::ArrayIndex i = 0; i < repeatDays.size(); ++i)
			{
				if (repeatDays[i] == "EACHMONDAY")
					mon = 1;
				else if (repeatDays[i] == "EACHTUESDAY")
					tue = 1;
				else if (repeatDays[i] == "EACHWEDNESDAY")
					wed = 1;
				else if (repeatDays[i] == "EACHTHUSDAY")
					thu = 1;
				else if (repeatDays[i] == "EACHFRIDAY")
					fri = 1;
				else if (repeatDays[i] == "EACHSATURDAY")
					sat = 1;
				else if (repeatDays[i] == "EACHSUNDAY")
					sun = 1;
			}
			repeat = Util::ConvertRepeatDayToInt(mon, tue, wed, thu, fri, sat, sun);
		}
		else
		{
			repeat = Util::ConvertRepeatDayToInt(1, 1, 1, 1, 1, 1, 1);
		}

		Rule *rule = NULL;

		// TODO: Check Type of Rule:
		/*
			- -1: Rule Time
			- +0: Rule OR
			- +1: Rule AND
			- +2: Rule Time + OR
			- +3: Rule Time + AND
		*/
		int logical = ruleValue["LOGICAL_OPERATOR_ID"].asInt();
		string type;
		if (logical == -1 || logical == 3 || logical == 2) // rule theo thoi gian or theo thoi gian va tb dau vao
		{
			if (logical == -1 || logical == 3)
			{
				type = "and";
			}
			else if (logical == 2)
			{
				type = "or";
			}
			if (ruleValue.isMember("START_AT") && ruleValue["START_AT"].isString())
			{
				string endAt;
				if (ruleValue.isMember("END_AT") && ruleValue["END_AT"].isString())
				{
					endAt = ruleValue["END_AT"].asString();
				}
				string startAt = ruleValue["START_AT"].asString();
				rule = new Rule(id, type, repeat, Util::ConvertStrTimeToInt(startAt), Util::ConvertStrTimeToInt(endAt));
				if (!rule)
				{
					LOGW("New rule error");
				}
			}
			else
			{
				LOGW("Event time not enough info");
			}
		}
		else if (logical == 0 || logical == 1) // rule theo thiet bi dau vao, logical or
		{
			if (logical == 0)
			{
				type = "or";
			}
			else if (logical == 1)
			{
				type = "and";
			}
			rule = new Rule(id, type, repeat);
			if (!rule)
			{
				LOGW("New rule error");
			}
		}

		if (rule)
		{
			if (ruleValue.isMember("INPUT_DEVICES") && ruleValue["INPUT_DEVICES"].isArray())
			{
				Json::Value listDevInput = ruleValue["INPUT_DEVICES"];
				for (Json::ArrayIndex countDev = 0; countDev < listDevInput.size(); countDev++)
				{
					Json::Value devInput = listDevInput[countDev];
					if (devInput.isMember("DEVICE_ID") && devInput["DEVICE_ID"].isString() && devInput.isMember("DEVICE_ATTRIBUTE") && devInput["DEVICE_ATTRIBUTE"].isObject())
					{
						string devId = devInput["DEVICE_ID"].asString();
						Json::Value devAttribute = devInput["DEVICE_ATTRIBUTE"];
						Json::Value datasDevInput;
						if (devAttribute.isMember("ID") && devAttribute["ID"].isInt() && devAttribute.isMember("VALUE"))
						{
							int id = devAttribute["ID"].asInt();
							string op = "";
							Json::Value values = Json::arrayValue;
							if (devAttribute["VALUE"].isArray())
							{
								values = devAttribute["VALUE"];
								op = "<>";
							}
							else if (devAttribute["VALUE"].isInt())
							{
								values.append(devAttribute["VALUE"].asInt());
								op = "=";
							}
							datasDevInput["ID"] = id;
							datasDevInput["VALUE"] = values;
							datasDevInput["OP"] = op;
						}
						Device *deviceInputRule = getDeviceFromId(devId);
						if (deviceInputRule)
						{
							RuleInputDevice *ruleInputDevice = new RuleInputDevice(rule, deviceInputRule, datasDevInput);
							if (ruleInputDevice)
							{
								rule->AddRuleInput(ruleInputDevice);
							}
							else
							{
								LOGW("create rule input device error");
							}
						}
						else
						{
							LOGW("Device %s does not exsit", devId.c_str());
						}
					}
				}
			}
			if (ruleValue.isMember("OUTPUT_DEVICES") && ruleValue["OUTPUT_DEVICES"].isArray())
			{
				Json::Value listDevOutput = ruleValue["OUTPUT_DEVICES"];
				for (Json::ArrayIndex countDevOp = 0; countDevOp < listDevOutput.size(); countDevOp++)
				{
					Json::Value devOutput = listDevOutput[countDevOp];
					if (devOutput.isMember("DEVICE_ID") && devOutput["DEVICE_ID"].isString() && devOutput.isMember("PROPERTIES") && devOutput["PROPERTIES"].isArray())
					{
						string devIdOp = devOutput["DEVICE_ID"].asString();
						Json::Value property = devOutput["PROPERTIES"];
						Device *deviceOutputRule = getDeviceFromId(devIdOp);
						if (deviceOutputRule)
						{
							RuleOutputDevice *ruleOutoutDevice = new RuleOutputDevice(deviceOutputRule, property);
							if (ruleOutoutDevice)
							{
								rule->AddRuleOutput(ruleOutoutDevice);
							}
							else
							{
								LOGW("Create rule output device error");
							}
						}
						else
						{
							LOGW("Device %s does not exsit", devIdOp.c_str());
						}
					}
				}
			}
			if (ruleValue.isMember("OUTPUT_GROUPS") && ruleValue["OUTPUT_GROUPS"].isArray())
			{
				Json::Value listGroupOutput = ruleValue["OUTPUT_GROUPS"];
				for (Json::ArrayIndex countGrp = 0; countGrp < listGroupOutput.size(); countGrp++)
				{
					Json::Value groupOutput = listGroupOutput[countGrp];
					if (groupOutput.isMember("GROUP_ID") && groupOutput["GROUP_ID"].isString() && groupOutput.isMember("PROPERTIES") && groupOutput["PROPERTIES"].isArray())
					{
						string groupId = groupOutput["GROUP_ID"].asString();
						Json::Value property = groupOutput["PROPERTIES"];
						Group *groupOutputRule = getGroupFromId(groupId);
						if (groupOutputRule)
						{
							RuleOutputGroup *ruleOutputGroup = new RuleOutputGroup(groupOutputRule, property);
							if (ruleOutputGroup)
							{
								rule->AddRuleOutput(ruleOutputGroup);
							}
							else
							{
								LOGW("Create rule output group error");
							}
						}
						else
						{
							LOGW("Group %s does not exsit", groupId.c_str());
						}
					}
				}
			}
			if (ruleValue.isMember("OUTPUT_SCENES") && ruleValue["OUTPUT_SCENES"].isArray())
			{
				Json::Value listScenesOutputRule = ruleValue["OUTPUT_SCENES"];
				for (Json::ArrayIndex countScene = 0; countScene < listScenesOutputRule.size(); countScene++)
				{
					Json::Value scenesOutputRule = listScenesOutputRule[countScene];
					if (scenesOutputRule.isObject())
					{
						if (scenesOutputRule.isMember("SCENE_ID") && scenesOutputRule["SCENE_ID"].isString())
						{
							string sceneId = scenesOutputRule["SCENE_ID"].asString();
							SceneBle *sceneOutputRule = getSceneBleFromId(sceneId);
							if (sceneOutputRule)
							{
								RuleOutputSceneBle *ruleOutputSceneBle = new RuleOutputSceneBle(sceneOutputRule);
								if (ruleOutputSceneBle)
								{
									rule->AddRuleOutput(ruleOutputSceneBle);
								}
								else
								{
									LOGW("Create rule output scene error");
								}
							}
							else
							{
								LOGW("Scene %s does not exsit", sceneId.c_str());
							}
						}
					}
				}
			}
			if (addGateway)
			{
				ruleList[id] = rule;
			}
			if (addDatabase)
			{
				string ruleStr = ruleValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->RuleAdd(id, ruleStr, status, 1);
			}
			bool isEnable = (status) ? true : false;
			rule->isEnable = isEnable;
		}
		return rule;
	}
	else
	{
		LOGW("Rule error format");
	}
	return NULL;
}

// Scene *Gateway::HandleRule

SceneBle *Gateway::AddNewSceneBle(SceneBle *sceneBle, bool addGateway, bool addDatabase)
{
	if (sceneBle)
	{
		if (addDatabase)
		{
			// int rs = database->GroupAdd(group);
			// if (rs)
			// {
			// LOGW("rs: %d", rs);
			// return NULL;
			// }
		}
		if (addGateway)
			sceneBleList[sceneBle->GetId()] = sceneBle;
	}
	return sceneBle;
}

Room *Gateway::AddNewRoom(Room *room)
{
	if (room)
	{
		roomList[room->GetUUId()] = room;
	}
	return room;
}

uint16_t Gateway::getBleUnicast()
{
	return ble_unicast;
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
	return "";
}
string Gateway::getRefreshToken()
{
	return refresh_token;
}

string Gateway::getMac()
{
	return mac;
}

void Gateway::setBleUnicast(uint16_t unicast)
{
	this->ble_unicast = unicast;
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
void Gateway::setRefreshToken(string refresh_token)
{
	this->refresh_token = refresh_token;
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
