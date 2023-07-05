#pragma once

#include "json.h"
#include <map>
#include <atomic>

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
	typedef struct
	{
		bool status;
		string respCmd;
		Json::Value *respValue;
	} request_t;
	map<string, request_t *> requestList;

	string mac;

	string pubReqTopic;
	string pubRespTopic;

	atomic<bool> isBusy;

	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnLocalCallbackFunc;
	map<string, OnLocalCallbackFunc> onLocalCallbackFuncList;

	void OnLocalMessage(string &topic, string &payload);
	void OnLocalReq(string &topic, string &payload);
	void OnLocalResp(string &topic, string &payload);

public:
	LocalProtocol(string mac, string address, int port, string token, string username, string password, int keepalive);
	virtual ~LocalProtocol();

	void init();
	void localAddActionCallback(ActionCallbackFuncType1 actionCallbackFuncType1, string topic);
	void localAddActionCallback(ActionCallbackFuncType2 actionCallbackFuncType2, string topic);
	void localAddActionCallback(ActionCallbackFuncType3 actionCallbackFuncType3, string topic);
	void localAddActionCallback(ActionCallbackFuncType4 actionCallbackFuncType4, string topic);

	bool IsBusy() { return isBusy; }

	int LocalConnect();
	void OnConnect(bool isConnected, bool isReconnect);
	virtual void OnLocalConnect(bool isConnected, bool isReconnect) {}

	int OnLocalCallbackRegister(string cmd, OnLocalCallbackFunc onLocalCallbackFunc);

	int LocalPublish(string topic, string payload);
	int LocalPublish(string topic, char *payload, int payloadLen);
	int LocalPublish(string &payload);
	int LocalPublish(Json::Value &payloadJson);

	int PublishToLocalMessageV2(string &payload);
	int PublishToLocalMessageV2(Json::Value &payloadJson);
	int PublishToLocalMessageV2(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout = 1000);
};