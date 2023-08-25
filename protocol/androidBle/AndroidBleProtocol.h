#pragma once

#include <string>
#include <stdint.h>
#include <functional>
#include "json.h"

using namespace std;

class AndroidBleProtocol
{
private:
	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnAndroidBleProtocolCallbackFunc;
	map<string, OnAndroidBleProtocolCallbackFunc> onAndroidBleProtocolCallbackFuncList;

	int OnAndroidBleProtocolCallbackRegister(string cmd, OnAndroidBleProtocolCallbackFunc onAndroidBleProtocolCallbackFunc);
	void OnMessage(string &topic, string &payload);
	int SendMessage(string data);

	int OnBleInfo(Json::Value &reqValue, Json::Value &respValue);
	int OnNewDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnDeviceStatus(Json::Value &reqValue, Json::Value &respValue);

public:
	AndroidBleProtocol();
	virtual ~AndroidBleProtocol();

	void init();

	int StartScan();
	int StopScan();
};

extern AndroidBleProtocol *androidBleProtocol;
