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

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	string subReqTopicV2;
	string subRespTopicV2;
	string pubReqTopicV2;
	string pubRespTopicV2;
#endif // CONFIG_USE_MESSAGE_FORMAT_V2

	atomic<bool> isBusy;

	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnLocalCallbackFunc;
	map<string, OnLocalCallbackFunc> onLocalCallbackFuncList;
	map<string, OnLocalCallbackFunc> onLocalCallbackFuncListV2;

	void OnLocalMessage(string &topic, string &payload);
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	void OnLocalMessageV2(string &topic, string &payload);
	void OnLocalRespV2(string &topic, string &payload);
#endif // CONFIG_USE_MESSAGE_FORMAT_V2

public:
	LocalProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive);
	virtual ~LocalProtocol();

	void init();

	bool IsBusy() { return isBusy; }

	int LocalConnect();
	void OnConnect(bool isConnected, bool isReconnect);
	virtual void OnLocalConnect(bool isConnected, bool isReconnect) {}

	int LocalPublish(string topic, string payload);

	int OnLocalCallbackRegister(string cmd, OnLocalCallbackFunc onLocalCallbackFunc);
	int PublishToLocalMessage(string &payload);
	int PublishToLocalMessage(Json::Value &payloadJson);

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	int OnLocalCallbackRegisterV2(string cmd, OnLocalCallbackFunc onLocalCallbackFunc);
	int PublishToLocalMessageV2(string &payload);
	int PublishToLocalMessageV2(Json::Value &payloadJson);
#endif // CONFIG_USE_MESSAGE_FORMAT_V2

	vector<string> listMsgPush;
	int PublishToLocalMessageV2(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout = 1000);
};
