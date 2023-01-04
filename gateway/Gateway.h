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
	string version;
	thread *udpBroadcastThread;
	bool isUdpBroadcasting;

	map<string, Device *> deviceList;
	map<int, Group *> groupList;
	map<int, Rule *> ruleList;
	vector<Device *> scanDeviceList;

	void OnCloudConnect(bool isConnected, bool isReconnect);
	void OnLocalConnect(bool isConnected, bool isReconnect);

	int UdpBroadcastThread();

	int GatewayConnectToCloudNotice();

	int OnUdpScanHc(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcScanWifi(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcSetup(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcConnectCloud(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCBleStartScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleStopScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleResetFactory(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleAddDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleDelDevice(Json::Value &reqValue, Json::Value &respValue);

#ifdef CONFIG_ENABLE_ZIGBEE
	int OnRPCZigbeeStartScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCZigbeeStopScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCZigbeeResetFactory(Json::Value &reqValue, Json::Value &respValue);
#endif
	int OnRPCAddGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCUpdateGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDelGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDelDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCAddDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCAddTuyaDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDelAllDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCGetScanDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCAddRule(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDeleteRule(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCControlDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCControlGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCUpdateAllTelemetry(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCSSHRemote(Json::Value &reqValue, Json::Value &respValue);

public:
	Gateway(string mac, string server_address, int server_port, string token, string username, string password, int keepalive, string localIp, int localPort, string localUsername, string localPassword, int localKeepalive);
	void init();

	/**
	 * @brief Send udp broadcast message to app when HC enters pairing mode
	 *
	 */
	void StartUdpBroadcast();

	void AddDeviceToScanList(Device *scanDevice);
	Group *getGroup(int id);
	Group *getGroupFromId(string groupId);

	Device *getDevice(string mac);
	Device *getDeviceFromId(string deviceId);
	DeviceBle *getDeviceBleFromAddr(uint32_t addr);

#ifdef CONFIG_ENABLE_ZIGBEE
	DeviceZigbee *getDeviceZigbeeFromAddr(uint32_t addr);
#endif

	Device *AddNewDevice(string id, string name, string mac, uint32_t addr, uint32_t type, bool addGateway, bool addDatabase);
	Group *AddNewGroup(Group *group, bool addGateway, bool addDatabase);
	Rule *AddRule(Json::Value &ruleValue, bool addGateway, bool addDatabase);

	string getBleNetkey();
	string getBleAppKey();
	string getBleDeviceKey();
	string getDormitory();
	string getId();
	string getVersion();
	string getName();

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