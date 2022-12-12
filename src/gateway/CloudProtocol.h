#pragma once

#include "Mqtt.h"
#include "json.h"
#include <map>

using namespace std;

typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnRPCCallbackFunc;

class CloudProtocol : public Mqtt
{
private:
	map<string, OnRPCCallbackFunc> onRPCCallbackFuncList;

	void OnDeviceRPC(string &topic, string &payload);

public:
	CloudProtocol(string server_address, int server_port, string token, string username, string password, int keepalive);
	virtual ~CloudProtocol();

	void init();

	int OnDeviceRPCCallbackRegister(string method, OnRPCCallbackFunc onRPCCallbackFunc);

	int OnlineDevice(string deviceName);
	int OfflineDevice(string deviceName);

	int PublishToDeviceTelemetry(string payload);
	int PublishToDeviceAttributes(string payload);
	int PublishToGatewayTelemetry(string payload);
	int PublishToGatewayAttributes(string payload);

	int PublishToDeviceTelemetry(Json::Value payloadJson);
	int PublishToDeviceAttributes(Json::Value payloadJson);
	int PublishToGatewayTelemetry(Json::Value payloadJson);
	int PublishToGatewayAttributes(Json::Value payloadJson);
};
