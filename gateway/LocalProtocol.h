#pragma once

#include "json.h"
#include <map>

using namespace std;

#ifdef ESP_PLATFORM
#include "MqttBroker.h"
class LocalProtocol : public MqttBroker
#else
#include "Mqtt.h"
class LocalProtocol : public Mqtt
#endif
{
private:
	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnLocalCallbackFunc;
	map<string, OnLocalCallbackFunc> onLocalCallbackFuncList;

	void OnLocalMessage(string &topic, string &payload);

public:
	LocalProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive);
	virtual ~LocalProtocol();

	void init();

	int LocalConnect();
	void OnConnect(bool isConnected, bool isReconnect);
	virtual void OnLocalConnect(bool isConnected, bool isReconnect) {}

	int LocalPublish(string topic, string payload);

	int OnLocalCallbackRegister(string method, OnLocalCallbackFunc onLocalCallbackFunc);

	int PublishToLocalMessage(string payload);
	int PublishToLocalMessage(Json::Value payloadJson);

    vector <string> listMsgPush;
};
