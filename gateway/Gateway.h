#pragma once

#include <string>
#include <map>
#include <functional>
#include <json.h>
#include "CloudProtocol.h"
#include "Udp.h"
#include "Device.h"
#include "Group.h"
#include "Device.h"
#ifdef CONFIG_ENABLE_BLE
#include "DeviceBle.h"
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
#include "DeviceZigbee.h"
#endif
#ifdef CONFIG_ENABLE_LORA
#include "DeviceLora.h"
#endif

using namespace std;

class Gateway : public CloudProtocol, public Udp
{
private:
	string dormitoryId;

	map<string, Device *> deviceList;
	map<int, Group *> groupList;
	map<int, Scene *> sceneList;
	vector<Device *> scanDeviceList;

	void OnConnect(bool isConnected, bool isReconnect);

	int OnUdpScanHc(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcScanWifi(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcConnectWifi(Json::Value &reqValue, Json::Value &respValue);
	int OnUdpHcConnectCloud(Json::Value &reqValue, Json::Value &respValue);

#ifdef CONFIG_ENABLE_BLE
	int OnRPCBleStartScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleStopScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleResetFactory(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleAddDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCBleDelDevice(Json::Value &reqValue, Json::Value &respValue);
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
	int OnRPCZigbeeStartScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCZigbeeStopScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCZigbeeResetFactory(Json::Value &reqValue, Json::Value &respValue);
#endif
#ifdef CONFIG_ENABLE_LORA
	int OnRPCLoraStartScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCLoraStopScan(Json::Value &reqValue, Json::Value &respValue);
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
	int OnRPCAddScene(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDeleteScene(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCControlDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCControlGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCUpdateAllTelemetry(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCSSHRemote(Json::Value &reqValue, Json::Value &respValue);

public:
	Gateway(string server_address, int server_port, string token, string username, string password, int keepalive);
	void init();

	void AddDeviceToScanList(Device *scanDevice);
	Group *getGroup(int id);
	Group *getGroupFromId(string groupId);

	Device *getDevice(string mac);

	Device *getDeviceFromId(string deviceId);

#ifdef CONFIG_ENABLE_BLE
	DeviceBle *getDeviceBleFromAddr(uint32_t addr);
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
	DeviceZigbee *getDeviceZigbeeFromAddr(uint32_t addr);
#endif
#ifdef CONFIG_ENABLE_LORA
	DeviceLora *getDeviceLoraFromAddr(uint32_t addr);
#endif

	Device *AddNewDevice(string id, string name, string mac, uint32_t addr, uint32_t type, bool addGateway, bool addDatabase);
	Group *AddNewGroup(Group *group, bool addGateway, bool addDatabase);
	Scene *AddScene(Json::Value &sceneValue, bool addGateway, bool addDatabase);

	void OnTimerTest();
	void PushRelayState(uint8_t relay);
};

extern Gateway *gateway;