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

	string mac;
	string subTopicV1;
	string pubTopicV1;

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	string subReqTopicV2;
	string subRespTopicV2;
	string pubReqTopicV2;
	string pubRespTopicV2;

	string subBinRespTopicV2;
	string pubBinReqTopicV2;
#endif // CONFIG_USE_MESSAGE_FORMAT_V2

	atomic<bool> isBusy;

	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnRpcCallbackFunc;
	map<string, OnRpcCallbackFunc> onRpcCallbackFuncList;
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	map<string, OnRpcCallbackFunc> onRpcCallbackFuncListV2;
#endif

	void OnDeviceRpc(string &topic, string &payload);

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	void OnDeviceRpcV2(string &topic, string &payload);
	void OnServerRespV2(string &topic, string &payload);
	void OnServerBinRespV2(string &topic, char *payload, int payloadLen);
#endif // CONFIG_USE_MESSAGE_FORMAT_V2

public:
	CloudProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive);
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

	int OnDeviceRpcCallbackRegister(string cmd, OnRpcCallbackFunc onRpcCallbackFunc);

	int OnlineHC(string deviceName);

	int CloudPublish(string topic, string payload);
	int CloudPublish(string topic, char *payload, int payloadLen);

	int PublishToDeviceTelemetry(string payload);
	int PublishToDeviceAttributes(string payload);
	int PublishToGatewayTelemetry(string payload);
	int PublishToGatewayAttributes(string payload);

	int PublishToDeviceTelemetry(Json::Value payloadJson);
	int PublishToDeviceAttributes(Json::Value payloadJson);
	int PublishToGatewayTelemetry(Json::Value payloadJson);
	int PublishToGatewayAttributes(Json::Value payloadJson);

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	int OnDeviceRpcCallbackRegisterV2(string cmd, OnRpcCallbackFunc onRpcCallbackFunc);
	int PublishToCloudMessageV2(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout = 5000);
	int PublishBinToCloudMessageV2(string sessionId, int index, char *payload, int payloadLen, string respCmd, Json::Value *respValue, uint32_t timeout = 5000);
	int PublishToCloudRecieveBinMessageV2(string reqCmd, Json::Value &reqValue, string rqi, char *payload, int *payloadLen, uint32_t timeout = 5000);
#endif // CONFIG_USE_MESSAGE_FORMAT_V2
};
