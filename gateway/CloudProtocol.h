#pragma once

#include "Mqtt.h"
#include "json.h"
#include <map>

using namespace std;

class CloudProtocol : public Mqtt
{
private:
	string subTopic;
	string pubTopic;
	string subTopicV2;
	string pubTopicV2;

	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnRpcCallbackFunc;
	map<string, OnRpcCallbackFunc> onRpcCallbackFuncList;
	typedef function<int(Json::Value &reqValue, Json::Value &respValue, string &rqi)> OnRpcCallbackFuncV2;
	map<string, OnRpcCallbackFuncV2> onRpcCallbackFuncListV2;

	void OnDeviceRpc(string &topic, string &payload);
	void OnDeviceRpcV2(string &topic, string &payload);

public:
	CloudProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive);
	virtual ~CloudProtocol();

	void init();
	void cloudAddActionCallback(ActionCallbackFuncType1 actionCallbackFuncType1, string topic);
	void cloudAddActionCallback(ActionCallbackFuncType2 actionCallbackFuncType2, string topic);
	void cloudAddActionCallback(ActionCallbackFuncType3 actionCallbackFuncType3, string topic);
	void cloudAddActionCallback(ActionCallbackFuncType4 actionCallbackFuncType4, string topic);

	int CloudConnect();
	void OnConnect(bool isConnected, bool isReconnect);
	virtual void OnCloudConnect(bool isConnected, bool isReconnect) {}

	int OnDeviceRpcCallbackRegister(string cmd, OnRpcCallbackFunc onRpcCallbackFunc);
	int OnDeviceRpcCallbackRegisterV2(string cmd, OnRpcCallbackFuncV2 onRpcCallbackFuncV2);

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
};
