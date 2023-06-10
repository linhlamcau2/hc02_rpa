#pragma once

#include "Mqtt.h"
#include "json.h"
#include <map>
#include <atomic>

using namespace std;

class CloudProtocol : public Mqtt
{
private:
	typedef struct
	{
		bool status;
		string respCmd;
		Json::Value *respValue;
	} request_t;
	map<string, request_t *> requestList;

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	typedef struct
	{
		bool status;
		char *payload;
		int *payloadLen;
	} request_bin_t;
	map<string, request_bin_t *> requestBinList;
#endif

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	string subReqTopic;
	string subRespTopic;
	string pubReqTopic;
	string pubRespTopic;

	string subBinRespTopic;
	string pubBinReqTopic;
#else
	string subTopic;
	string pubTopic;
#endif // CONFIG_USE_MESSAGE_FORMAT_V2

	string mac;
	atomic<bool> isBusy;
	atomic<bool> isConfig;

	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnRpcCallbackFunc;
	map<string, OnRpcCallbackFunc> onRpcCallbackFuncList;

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	void OnServerReq(string &topic, string &payload);
	void OnServerResp(string &topic, string &payload);
	void OnServerBinResp(string &topic, char *payload, int payloadLen);
#else
	void OnDeviceRpc(string &topic, string &payload);
#endif // CONFIG_USE_MESSAGE_FORMAT_V2

public:
	CloudProtocol(string mac, string address, int port, string clientId, string username, string password, int keepalive);
	virtual ~CloudProtocol();

	void init();

	bool IsBusy() { return isBusy; }
	bool IsConfig() { return isConfig; }
	void SetConfig(bool value);

	void cloudAddActionCallback(ActionCallbackFuncType1 actionCallbackFuncType1, string topic);
	void cloudAddActionCallback(ActionCallbackFuncType2 actionCallbackFuncType2, string topic);
	void cloudAddActionCallback(ActionCallbackFuncType3 actionCallbackFuncType3, string topic);
	void cloudAddActionCallback(ActionCallbackFuncType4 actionCallbackFuncType4, string topic);

	int CloudConnect();
	void OnConnect(bool isConnected, bool isReconnect);
	virtual void OnCloudConnect(bool isConnected, bool isReconnect) {}

	int OnDeviceRpcCallbackRegister(string cmd, OnRpcCallbackFunc onRpcCallbackFunc);

	int OnlineHC(string deviceName);

	int CloudPublish(string topic, string payload);
	int CloudPublish(string topic, char *payload, int payloadLen);
	int CloudPublish(string payload);
	int CloudPublish(Json::Value payloadJson);

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	int PublishToCloudMessageV2(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout = 5000);
	int PublishBinToCloudMessageV2(string sessionId, int index, char *payload, int payloadLen, string respCmd, Json::Value *respValue, uint32_t timeout = 5000);
	int PublishToCloudRecieveBinMessageV2(string reqCmd, Json::Value &reqValue, string rqi, char *payload, int *payloadLen, uint32_t timeout = 5000);
#endif // CONFIG_USE_MESSAGE_FORMAT_V2
};
