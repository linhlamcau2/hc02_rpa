#pragma once

#include <string>
#include <map>
#include <functional>
#include <thread>
#include <mutex>
#include "json.h"
#include "Define.h"
#include "ErrorCode.h"
#include "CloudProtocol.h"
#include "LocalProtocol.h"
#include "Udp.h"
#include "Device.h"
#include "Group.h"
#include "Device.h"
#include "DeviceBle.h"
#include "SceneBle.h"
#include "RuleInputTimer.h"
#include "RuleOutputSceneBle.h"
#include "RuleOutputDevice.h"
#include "RuleOutputGroup.h"
#include "RuleOutputDelay.h"
#include "Room.h"

#include "BleDefine.h"
#include "BleProtocol.h"

#ifdef CONFIG_ENABLE_ZIGBEE
#include "DeviceZigbee.h"
#endif

#define STR_(x) #x
#define STR(x) STR_(x)

using namespace std;

class Gateway : public CloudProtocol, public LocalProtocol, public Udp
{
private:
	string id;
	string mac;
	string version;
	string ble_netkey;
	string ble_appkey;
	string ble_devicekey;
	uint16_t ble_addr;
	uint32_t ble_iv_index;
	string dormitoryId;
	string refresh_token;
	string data;
	thread *udpBroadcastThread;
	atomic<bool> isUdpBroadcasting;

	bool isInternet;

	map<string, Device *> deviceList;
	map<string, Group *> groupList;
	map<string, SceneBle *> sceneBleList;
	map<string, Rule *> ruleList;
	map<string, Room *> roomList;

	mutex deviceListMtx;
	mutex groupListMtx;
	mutex ruleListMtx;
	mutex roomListMtx;
	mutex sceneBleListMtx;

	void OnCloudConnect(bool isConnected, bool isReconnect);
	void OnLocalConnect(bool isConnected, bool isReconnect);

	int GatewayConnectToCloudNotice();

	// Udp message handle
	void InitUdpMessage();
	int OnUdpScanHc(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcScanWifi(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcSetup(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcConnectCloud(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcInfo(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcSetPwMqttOnline(Json::Value &reqValue, Json::Value &respValue);

	// Device
	void InitMqttMessageDevice();
	int OnControlDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnControlAllDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnGetDeviceStatus(Json::Value &reqValue, Json::Value &respValue);
	int OnGetAllDeviceStatus(Json::Value &reqValue, Json::Value &respValue);
	int OnGetDeviceList(Json::Value &reqValue, Json::Value &respValue);
	int OnGetCamList(Json::Value &reqValue, Json::Value &respValue);
	int OnGetAllCam(Json::Value &reqValue, Json::Value &respValue);
	int OnNewDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnAddFavoriteDev(Json::Value &reqValue, Json::Value &respValue);
	int OnDelFavoriteDev(Json::Value &reqValue, Json::Value &respValue);
	int OnGetFavoriteDev(Json::Value &reqValue, Json::Value &respValue);
	int OnUpdateDeviceName(Json::Value &reqValue, Json::Value &respValue);

	int OnCreateSwitchLink(Json::Value &reqValue, Json::Value &respValue);
	int OnAddBtToSwitchLink(Json::Value &reqValue, Json::Value &respValue);
	int OnDelBtFromSwitchLink(Json::Value &reqValue, Json::Value &respValue);
	int OnDelSwitchLink(Json::Value &reqValue, Json::Value &respValue);

	// Group
	void InitMqttMessageGroup();
	int OnControlGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnGetGroupList(Json::Value &reqValue, Json::Value &respValue);
	int OnGetDevListInGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnAddDeviceGroupBle(Json::Value &deviceList, Json::Value &respSuccessList, Json::Value &respFailList, Group *group);
	int OnCreateGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnDelDeviceGroupBle(Json::Value &deviceList, Json::Value &respSuccessList, Json::Value &respFailList, Group *group);
	int OnDeleteDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnUpdateGroupName(Json::Value &reqValue, Json::Value &respValue);

	// Room
	void InitMqttMessageRoom();
	int OnGetRoomList(Json::Value &reqValue, Json::Value &respValue);
	int OnGetDevListInRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnCreateRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnAddDeviceToRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteDeviceFromRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnGetGroupIntoRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnGetSceneIntoRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnUpdateRoomName(Json::Value &reqValue, Json::Value &respValue);
	int OnCheckRoom(Json::Value &reqValue, Json::Value &respValue);

	// Rule
	void InitMqttMessageRule();
	int OnGetRuleList(Json::Value &reqValue, Json::Value &respValue);
	int OnGetRuleInfo(Json::Value &reqValue, Json::Value &respValue);
	int OnCreateRule(Json::Value &reqValue, Json::Value &respValue);
	int OnEditRule(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteRule(Json::Value &reqValue, Json::Value &respValue);
	int OnActiveRule(Json::Value &reqValue, Json::Value &respValue);
	int OnActionRule(Json::Value &reqValue, Json::Value &respValue);

	// Scene
	void InitMqttMessageScene();
	int OnControlScene(Json::Value &reqValue, Json::Value &respValue);
	int OnGetSceneList(Json::Value &reqValue, Json::Value &respValue);
	int OnGetDevListInScene(Json::Value &reqValue, Json::Value &respValue);
	int OnCreateScene(Json::Value &reqValue, Json::Value &respValue);
	int OnEditScene(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteScene(Json::Value &reqValue, Json::Value &respValue);
	int OnCallScene(Json::Value &reqValue, Json::Value &respValue);
	int OnAddDevToScene(Json::Value &reqValue, Json::Value &respValue);
	int OnDelDevToScene(Json::Value &reqValue, Json::Value &respValue);

	int ConfigSceneForRemote(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene);
	int ConfigSceneForPirSensor(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene);
	int ConfigSceneForScreenTouch(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene);
	int OnCreateSceneController(Json::Value &reqValue, Json::Value &respValue);
	int OnDelSceneController(Json::Value &reqValue, Json::Value &respValue);
	int OnAddFavoriteScene(Json::Value &reqValue, Json::Value &respValue);
	int OnDelFavoriteScene(Json::Value &reqValue, Json::Value &respValue);
	int OnGetFavoriteScene(Json::Value &reqValue, Json::Value &respValue);
	int OnUpdateSceneName(Json::Value &reqValue, Json::Value &respValue);

	// Hc
	void InitMqttMessageHc();
	int OnControlHc(Json::Value &reqValue, Json::Value &respValue);
	int OnGetHcInfo(Json::Value &reqValue, Json::Value &respValue);
	int OnStartScanBle(Json::Value &reqValue, Json::Value &respValue);
	int OnStopScanBle(Json::Value &reqValue, Json::Value &respValue);
	int OnResetHC(Json::Value &reqValue, Json::Value &respValue);
	int OnVersionHC(Json::Value &reqValue, Json::Value &respValue);
	int OnSSHRemote(Json::Value &reqValue, Json::Value &respValue);
	int OnCreateTunnel(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteAllTunnel(Json::Value &reqValue, Json::Value &respValue);
	int OnOtaHc(Json::Value &reqValue, Json::Value &respValue);
	int OnSetPasswordMqtt(Json::Value &reqValue, Json::Value &respValue);

public:
	Gateway(string mac, string address, int port, string clientId, string username, string password, int keepalive, char *cert, string localAddress = "localhost", int localPort = 1883, string localUsername = "", string localPassword = "", int localKeepalive = 10);
	~Gateway();
	void init();

	/**
	 * @brief Factory reset (call when hold reset button in 5s)
	 *
	 */
	void ResetFactory();
	void DelDatabase();

	/**
	 * @brief Send udp broadcast message to app when HC enters pairing mode
	 *
	 */
	void StartUdpBroadcast();
	void StopUdpBroadcast();
	int UdpBroadcastThread();

	int CheckOnlineThread();

	void AddDeviceToScanList(Device *scanDevice);

	Device *getDeviceFromMac(string mac);
	Device *getDeviceFromId(string id);
	DeviceBle *getDeviceBleFromAddr(uint16_t addr);
#ifdef CONFIG_ENABLE_ZIGBEE
	DeviceZigbee *getDeviceZigbeeFromAddr(uint16_t addr);
#endif
	void delDevice(Device *device);

	Group *getGroupFromId(string id);
	Group *getGroupFromAddr(uint16_t addr);
	void delGroup(Group *group);
	uint16_t getNextGroupAddr();

	SceneBle *getSceneBleFromId(string id);
	SceneBle *getSceneBleFromAddr(uint16_t addr);
	void delSceneBle(SceneBle *sceneBle);
	uint16_t getNextSceneBleAddr();

	Rule *getRuleFromId(string id);
	void delRule(Rule *rule);

	Room *getRoomFromId(string id);
	void delRoom(Room *room);
	uint16_t getNextRoomAddr();

	uint16_t getBleAddr();
	uint32_t getBleIvIndex();
	string getBleNetKey();
	string getBleAppKey();
	string getBleDeviceKey();
	string getDormitory();
	string getId();
	string getVersion();
	string getName();
	string getRefreshToken();
	string getData();
	string getMac();

	void setBleAddr(uint16_t addr);
	void setBleIvIndex(uint32_t ivIndex);
	void setBleNetkey(string netkey);
	void setBleAppkey(string appkey);
	void setBleDevicekey(string devicekey);
	void setDormitory(string dormitory);
	void setId(string id);
	void setMac(string mac);
	void setVersion(string version);
	void setName(string name);
	void setRefreshToken(string refresh_token);
	void setData(string data);

	void DelAllDevice();
	void DelAllGroup();
	void DelAllSceneBle();
	void DelAllRule();
	void DelAllRoom();

	void OnTimerTest();
	void PushRelayState(uint8_t relay);

	Device *AddNewDevice(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version, bool addDatabase);
	Group *AddNewGroup(Group *group, bool addDatabase);
	SceneBle *AddNewSceneBle(SceneBle *sceneBle, bool addDatabase);
	Room *AddNewRoom(Room *room, bool addDatabase);
	Rule *AddRule(Json::Value &ruleValue, bool addDatabase);

	int Do(Json::Value &dataValue);

	int pushDeviceUpdateCloud(Json::Value &dataValue);
	int pushNewDeviceLocal(Json::Value &dataValue);
	int pushDeviceUpdateLocal(Json::Value &dataValue);
	int pushNewDeviceCloud(Json::Value &dataValue);
	int pushStartAddHc(Json::Value &dataValue);
	int pushStopAddHc(Json::Value &dataValue);
	int pushMsgHcCoreToHcApp(string cmd, string id, string name, Json::Value &listDevice, string roomId);
	string CreateJsonGroupSceneSendHcCoreToHcApp(string cmd, string id, string name, Json::Value &listDevice, string roomId);

	// debug
	void printGroup();
	void printScene();
	void printRoom();
};

extern Gateway *gateway;
