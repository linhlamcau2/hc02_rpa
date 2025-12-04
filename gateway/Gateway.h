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
#include "Device.h"
#include "DeviceBle.h"
#include "Util.h"

#include "BleDefine.h"
#include "BleProtocol.h"

using namespace std;

class Gateway : public CloudProtocol, public LocalProtocol
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

	void OnCloudConnect(bool isConnected, bool isReconnect);
	void OnLocalConnect(bool isConnected, bool isReconnect);

public:
	Gateway(string mac, string address, int port, string clientId, string username, string password, int keepalive, string localAddress = "localhost", int localPort = 1883, string localUsername = "", string localPassword = "", int localKeepalive = 10);
	~Gateway();
	void init();

	void ResetFactory();
	void DelDatabase();

	int RestartBleGw();
	int TestSwitch();


	Device *getDeviceFromMac(string mac);
	Device *getDeviceFromId(string id);
	DeviceBle *getDeviceBleFromAddr(uint16_t addr);
	void delDevice(Device *device);

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
	bool getAutoOta();

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
	void setAutoOta(bool isAutoOta);


	int pushDeviceUpdateCloud(Json::Value &dataValue);
	int pushNewDeviceLocal(Json::Value &dataValue);
	int pushDeviceUpdateLocal(Json::Value &dataValue);
	int pushNewDeviceCloud(Json::Value &dataValue);
	int pushStartAddHc(Json::Value &dataValue);
	int pushStopAddHc(Json::Value &dataValue);
	int pushNotify(Json::Value &dataValue);
};

extern Gateway *gateway;

void active_test_pcba_dhpt(const string& rqi_recv,const string& serial_recv,const string& ver_recv);
