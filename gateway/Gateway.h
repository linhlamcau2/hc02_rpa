#pragma once

#include <string>
#include <map>
#include <functional>
#include <json.h>
#include "CloudProtocol.h"
#include "LocalProtocol.h"
#include "Udp.h"
#include "Device.h"
#include "Group.h"
#include "Device.h"
#include "DeviceBle.h"
#include "SceneBle.h"
#include "RuleOutputSceneBle.h"
#include "../room/Room.h"

#ifdef CONFIG_ENABLE_ZIGBEE
#include "DeviceZigbee.h"
#endif

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
	uint16_t ble_unicast;
	string version;
	thread *udpBroadcastThread;
	bool isUdpBroadcasting;

	map<string, Device *> deviceList;
	map<int, Group *> groupList;
	map<string, Rule *> ruleList;
	map<int, SceneBle *> sceneBleList;
	map<string, Room *> roomList;
	vector<Device *> scanDeviceList;

	void OnCloudConnect(bool isConnected, bool isReconnect);
	void OnLocalConnect(bool isConnected, bool isReconnect);

	int CheckOnlineThread();
	int UdpBroadcastThread();

	int GatewayConnectToCloudNotice();

	int OnUdpScanHc(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcScanWifi(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcSetup(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcConnectCloud(Json::Value &reqValue, Json::Value &respValue);

    int OnRPCHcConnectCloud(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleStartScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleStopScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleReset(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleAddDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleDelDevice(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCResetFactory(Json::Value &reqValue, Json::Value &respValue);

#ifdef CONFIG_ENABLE_ZIGBEE
	int OnRPCZigbeeStartScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCZigbeeStopScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCZigbeeResetFactory(Json::Value &reqValue, Json::Value &respValue);
#endif
	int OnRPCCreateRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCAddDevToRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCRemoveDevFromRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDeleteRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCCheckRoom(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCAddGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCUpdateGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDelGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDelDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCSetSceneForRemote(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDelSceneForRemote(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCResetRemote(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCScenePirLigtSensor(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCEditScenePirLightSensor(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCRemoveScenePirLightSensor(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCSceneScreen(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCStairsSwitch(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCEditStairsSwitch(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDelStairsSwitch(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCAddDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCAddTuyaDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDelAllDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCGetScanDevice(Json::Value &reqValue, Json::Value &respValue);

	// Rule
	int OnRPCAddRule(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCEditRule(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCSwitchStatusEvent(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDeleteRule(Json::Value &reqValue, Json::Value &respValue);

	//HCL
	int OnRPCCreateHCL(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDeleteHCL(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCEditHCL(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCSwitchStatusHCL(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCAddSceneBle(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCEditSceneBle(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDeleteSceneBle(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCControlDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCControlGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCUpdateAllTelemetry(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCControlSceneBle(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCSSHRemote(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCSetPwMqttOnline(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCAddDeviceSmartHomeToRoom(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCRemoveDeviceSmartHomeToRoom(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCCreateCountDown(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDelCountDown(Json::Value &reqValue, Json::Value &respValue);

public:
	Gateway(string mac, string server_address, int server_port, string token, string username, string password, int keepalive, string localIp, int localPort, string localUsername, string localPassword, int localKeepalive);
	void init();
	/**
	 * @brief Send udp broadcast message to app when HC enters pairing mode
	 *
	 */
	void StartUdpBroadcast();
	void StopUdpBroadcast();

	void AddDeviceToScanList(Device *scanDevice);
	Group *getGroup(int id);
	Group *getGroupFromId(string groupId);

	Device *getDevice(string mac);
	Device *getDeviceFromId(string deviceId);
	DeviceBle *getDeviceBleFromAddr(uint32_t addr);
	Rule *getRuleById(string eventId);

	SceneBle *getSceneBleFromId(string sceneBleUUId);
	Room *getRoomFromId(string roomUUId);

#ifdef CONFIG_ENABLE_ZIGBEE
	DeviceZigbee *getDeviceZigbeeFromAddr(uint32_t addr);
#endif

	Device *AddNewDevice(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version, bool addGateway, bool addDatabase);
	Group *AddNewGroup(Group *group, bool addGateway, bool addDatabase);
	Rule *AddRule(Json::Value &ruleValue, bool addGateway, bool addDatabase);
	SceneBle *AddNewSceneBle(SceneBle *sceneBle, bool addGateway, bool addDatabase);
	Room *AddNewRoom(Room *room);

	uint16_t getBleUnicast();
	string getBleNetkey();
	string getBleAppKey();
	string getBleDeviceKey();
	string getDormitory();
	string getId();
	string getVersion();
	string getName();

	void setBleUnicast(uint16_t unicast);
	void setBleNetkey(string netkey);
	void setBleAppkey(string appkey);
	void setBleDevicekey(string devicekey);
	void setDormitory(string dormitory);
	void setId(string id);
	void setVersion(string version);
	void setName(string name);
	void OnTimerTest();
	void PushRelayState(uint8_t relay);
};

extern Gateway *gateway;
