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
		string pubTopic;
	} request_t;
	map<string, request_t *> requestList;

	string subServerReqTopic;
	string pubServerReqTopic;

	string mac;
	atomic<bool> isBusy;

	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnRpcCallbackFunc;
	map<string, OnRpcCallbackFunc> onRpcCallbackFuncList;
	map<string, map<string, OnRpcCallbackFunc>> onRpcCmdCallbackFuncList;

	void OnServerReq(string &topic, string &payload);
	int OnServerCmdReq(Json::Value &reqValue, Json::Value &respValue);
	// void OnServerResp(string &topic, string &payload);
	// void OnMobileReq(string &topic, string &payload);
	// void OnMobileResp(string &topic, string &payload);
	// void OnServerBinResp(string &topic, char *payload, int payloadLen);

public:
	CloudProtocol(string mac, string address, int port, string clientId, string username, string password, int keepalive, char *cert);
	virtual ~CloudProtocol();

	void init();

	bool IsBusy() { return isBusy; }

	void cloudAddActionCallback(ActionCallbackFuncType1 actionCallbackFuncType1, string topic);
	void cloudAddActionCallback(ActionCallbackFuncType2 actionCallbackFuncType2, string topic);
	void cloudAddActionCallback(ActionCallbackFuncType3 actionCallbackFuncType3, string topic);
	void cloudAddActionCallback(ActionCallbackFuncType4 actionCallbackFuncType4, string topic);

	int CloudConnect();
	void OnConnect(bool isConnected, bool isReconnect);
	virtual void OnCloudConnect(bool isConnected, bool isReconnect) {}

	int OnDeviceRpcCallbackRegister(string type, OnRpcCallbackFunc onRpcCallbackFunc);
	int OnDeviceRpcCmdCallbackRegister(string command, string attribute, OnRpcCallbackFunc onRpcCallbackFunc);

	int OnlineHC(string deviceName);

	int CloudPublish(string topic, string payload);
	int CloudPublish(string topic, char *payload, int payloadLen);
	int CloudPublish(string payload);
	int CloudPublish(Json::Value payloadJson);

	int PublishToCloudMessage(string reqCmd, Json::Value &reqValue);
	int PublishToCloudMessage(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout = 0);
	int PublishBinToCloudMessage(string sessionId, int index, char *payload, int payloadLen, string respCmd, Json::Value *respValue, uint32_t timeout = 0);
	int PublishToCloudRecieveBinMessage(string reqCmd, Json::Value &reqValue, string rqi, char *payload, int *payloadLen, uint32_t timeout = 0);
};