#pragma once

#include <string>
#include <map>
#include <functional>
#include <thread>
#include "json.h"
#include "CloudProtocol.h"
#include "LocalProtocol.h"
#include "Udp.h"
#include "Device.h"
#include "Group.h"
#include "Device.h"
#include "DeviceBle.h"
#include "SceneBle.h"
#include "RuleOutputSceneBle.h"
#include "Room.h"
#include "ErrorCode.h"

#ifdef CONFIG_ENABLE_ZIGBEE
#include "DeviceZigbee.h"
#endif

#ifndef VERSION
#define VERSION 0.0.1
#endif

#define STR_(x) #x
#define STR(x) STR_(x)

using namespace std;

class Gateway : public CloudProtocol, public LocalProtocol, public Udp
{
private:
	string id;
	string mac;
	string dormitoryId;
	string ble_netkey;
	string ble_appkey;
	string ble_devicekey;
	string refresh_token;
	uint16_t ble_unicast;
	string version;
	thread *udpBroadcastThread;
	bool isUdpBroadcasting;

	map<string, Device *> deviceList;
	map<string, Group *> groupList;
	map<string, SceneBle *> sceneBleList;
	map<string, Rule *> ruleList;
	map<string, Room *> roomList;

	void OnCloudConnect(bool isConnected, bool isReconnect);
	void OnLocalConnect(bool isConnected, bool isReconnect);

	int CheckOnlineThread();
	int UdpBroadcastThread();

	int GatewayConnectToCloudNotice();

	// Udp message handle
	void initUdpMessage();
	int OnUdpScanHc(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcScanWifi(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcSetup(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcConnectCloud(Json::Value &reqValue, Json::Value &respValue);

	// Mqtt message handle
	void initMqttMessage();
	int OnRpcHcConnectCloud(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcHcBackup(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcBleStartScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcBleStopScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcBleReset(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcBleAddDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcBleDelDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcResetFactory(Json::Value &reqValue, Json::Value &respValue);

#ifdef CONFIG_ENABLE_ZIGBEE
	int OnRpcZigbeeStartScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcZigbeeStopScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcZigbeeResetFactory(Json::Value &reqValue, Json::Value &respValue);
#endif
	int OnRpcCreateRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcAddDevToRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcRemoveDevFromRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcDeleteRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcCheckRoom(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcAddGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcUpdateGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcDelGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcDelDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcSetSceneForRemote(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcDelSceneForRemote(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcResetRemote(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcScenePirLigtSensor(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcEditScenePirLightSensor(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcRemoveScenePirLightSensor(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcSceneScreen(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcStairsSwitch(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcEditStairsSwitch(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcDelStairsSwitch(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcAddDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcAddTuyaDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcDelAllDevice(Json::Value &reqValue, Json::Value &respValue);

	// Rule
	int OnRpcAddRule(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcEditRule(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcSwitchStatusEvent(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcDeleteRule(Json::Value &reqValue, Json::Value &respValue);

	// HCL
	int OnRpcCreateHCL(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcDeleteHCL(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcEditHCL(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcSwitchStatusHCL(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcAddSceneBle(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcEditSceneBle(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcDeleteSceneBle(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcControlDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcControlGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcUpdateAllTelemetry(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcControlSceneBle(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcSSHRemote(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcUpdateFirmware(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcSetPwMqttOnline(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcAddDeviceSmartHomeToRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcRemoveDeviceSmartHomeToRoom(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcCreateCountDown(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcDelCountDown(Json::Value &reqValue, Json::Value &respValue);

	// Mqtt V2 message handle
	void initMqttMessageV2();
	// Bản tin điều khiển
	int OnControlDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnControlAllDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnControlGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnControlScene(Json::Value &reqValue, Json::Value &respValue);
	int OnRequestDeviceStatus(Json::Value &reqValue, Json::Value &respValue);
	// Bản tin cấu hình
	int OnStartScanBle(Json::Value &reqValue, Json::Value &respValue);
	int OnStopScanBle(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnCreateGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnCreateScene(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteScene(Json::Value &reqValue, Json::Value &respValue);
	int OnCallScene(Json::Value &reqValue, Json::Value &respValue);
	// thieu scene controller
	int OnCreateRule(Json::Value &reqValue, Json::Value &respValue);
	int OnGetInfoHC(Json::Value &reqValue, Json::Value &respValue);

	// Cấu hình HC
	int OnResetHC(Json::Value &reqValue, Json::Value &respValue);

	int OnGetDeviceStatus(Json::Value &reqValue, Json::Value &respValue);
	int OnGetAllDeviceStatus(Json::Value &reqValue, Json::Value &respValue);
	int OnGetDeviceList(Json::Value &reqValue, Json::Value &respValue);

public:
	Gateway(string mac, string server_address, int server_port, string token, string username, string password, int keepalive, string localIp, int localPort, string localUsername, string localPassword, int localKeepalive);
	void init();

	/**
	 * @brief Factory reset (call when hold reset button in 5s)
	 *
	 */
	void ResetFactory();

	/**
	 * @brief Send udp broadcast message to app when HC enters pairing mode
	 *
	 */
	void StartUdpBroadcast();
	void StopUdpBroadcast();

	void AddDeviceToScanList(Device *scanDevice);

	Device *getDeviceFromMac(string mac);
	Device *getDeviceFromId(string id);
	DeviceBle *getDeviceBleFromAddr(uint32_t addr);

	Group *getGroupFromId(string id);
	Group *getGroupFromAddr(int addr);

	SceneBle *getSceneBleFromId(string id);
	SceneBle *getSceneBleFromAddr(int addr);

	Rule *getRuleFromId(string id);

	Room *getRoomFromId(string id);

	uint16_t getBleUnicast();
	string getBleNetkey();
	string getBleAppKey();
	string getBleDeviceKey();
	string getDormitory();
	string getId();
	string getVersion();
	string getName();
	string getRefreshToken();
	string getMac();

	void setBleUnicast(uint16_t unicast);
	void setBleNetkey(string netkey);
	void setBleAppkey(string appkey);
	void setBleDevicekey(string devicekey);
	void setDormitory(string dormitory);
	void setId(string id);
	void setVersion(string version);
	void setName(string name);
	void setRefreshToken(string refresh_token);
	void OnTimerTest();
	void PushRelayState(uint8_t relay);

	void AddAllDeviceStatusV2(Json::Value &reqValue);

	Device *AddNewDevice(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version, bool addGateway, bool addDatabase);
	Group *AddNewGroup(Group *group, bool addGateway, bool addDatabase);
	Rule *AddRule(Json::Value &ruleValue, bool addGateway, bool addDatabase);
	Rule *AddRuleV2(Json::Value &ruleValue);
	SceneBle *AddNewSceneBle(SceneBle *sceneBle, bool addGateway, bool addDatabase);
	Room *AddNewRoom(Room *room);
};

extern Gateway *gateway;
