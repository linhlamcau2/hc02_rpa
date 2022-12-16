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
	
	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnRPCCallbackFunc;
	map<string, OnRPCCallbackFunc> onRPCCallbackFuncList;

	void OnDeviceRPC(string &topic, string &payload);

public:
	CloudProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive);
	virtual ~CloudProtocol();

	void init();

	int CloudConnect(int timeout = 10);
	void OnConnect(bool isConnected, bool isReconnect);
	virtual void OnCloudConnect(bool isConnected, bool isReconnect) {}

	int OnDeviceRPCCallbackRegister(string method, OnRPCCallbackFunc onRPCCallbackFunc);

	int OnlineHC(string deviceName);

	int CloudPublish(string topic, string payload);

	int PublishToDeviceTelemetry(string payload);
	int PublishToDeviceAttributes(string payload);
	int PublishToGatewayTelemetry(string payload);
	int PublishToGatewayAttributes(string payload);

	int PublishToDeviceTelemetry(Json::Value payloadJson);
	int PublishToDeviceAttributes(Json::Value payloadJson);
	int PublishToGatewayTelemetry(Json::Value payloadJson);
	int PublishToGatewayAttributes(Json::Value payloadJson);
};
