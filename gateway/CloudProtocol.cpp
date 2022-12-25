#include "CloudProtocol.h"
#include <string.h>
#include <Log.h>

#define HC_ONLINE "/hc/online"
#define HC_OFFLINE "/hc/offline"

CloudProtocol::CloudProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive) : Mqtt(server_address, server_port, token, username, password, keepalive)
{
	subTopic = "/server/" + mac;
	pubTopic = "/" + mac + "/server";
	Json::Value jsonValue;
	jsonValue["HC_ID"] = mac;
	SetWillset(HC_OFFLINE, jsonValue.toString());
}

CloudProtocol::~CloudProtocol()
{
}

void CloudProtocol::init()
{
	addActionCallback(bind(&CloudProtocol::OnDeviceRPC, this, placeholders::_1, placeholders::_2), subTopic);
}

int CloudProtocol::CloudConnect()
{
	return Connect();
}

void CloudProtocol::OnConnect(bool isConnected, bool isReconnect)
{
	OnCloudConnect(isConnected, isReconnect);
}

void CloudProtocol::OnDeviceRPC(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	string errs;
	stringstream s(payload);
	Json::CharReaderBuilder b;
	Json::parseFromStream(b, s, &payloadJson, &errs);
	if (payloadJson.isMember("CMD") && payloadJson["CMD"].isString())
	{
		string method = payloadJson["CMD"].asString();
		if (onRPCCallbackFuncList.find(method) != onRPCCallbackFuncList.end())
		{
			OnRPCCallbackFunc onRPCCallbackFunc = onRPCCallbackFuncList[method];
			int rs = onRPCCallbackFunc(payloadJson, respValue);
			if (rs == 0)
			{
				LOGD("Call %s OK, rs: %d", method.c_str(), rs);
				Publish(pubTopic, respValue.toString());
			}
			else if (rs == 1)
			{
				LOGD("Call %s OK, rs: %d", method.c_str(), rs);
			}
			else
			{
				LOGW("Call %s ERR rs: %d", method.c_str(), rs);
			}
		}
		else
		{
			LOGW("Method %s not registed", method.c_str());
			LOGW("OnDeviceRPC payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnDeviceRPC topic: %s", topic.c_str());
		LOGW("OnDeviceRPC payload: %s", payload.c_str());
	}
}

int CloudProtocol::OnDeviceRPCCallbackRegister(string method, OnRPCCallbackFunc onRPCCallbackFunc)
{
	LOGI("OnDeviceRPCCallbackRegister method: %s", method.c_str());
	onRPCCallbackFuncList[method] = onRPCCallbackFunc;
	return 0;
}

int CloudProtocol::OnlineHC(string deviceName)
{
	Json::Value jsonValue;
	jsonValue["HC_ID"] = deviceName;
	return Publish(HC_ONLINE, jsonValue.toString());
}

int CloudProtocol::CloudPublish(string topic, string payload)
{
	return Publish(topic, payload);
}

int CloudProtocol::PublishToDeviceTelemetry(string payload)
{
	return Publish(pubTopic, payload);
}

int CloudProtocol::PublishToDeviceAttributes(string payload)
{
	return Publish(pubTopic, payload);
}

int CloudProtocol::PublishToGatewayTelemetry(string payload)
{
	return Publish(pubTopic, payload);
}

int CloudProtocol::PublishToGatewayAttributes(string payload)
{
	return Publish(pubTopic, payload);
}

int CloudProtocol::PublishToDeviceTelemetry(Json::Value payloadJson)
{
	return PublishToDeviceTelemetry(payloadJson.toString());
}

int CloudProtocol::PublishToDeviceAttributes(Json::Value payloadJson)
{
	return PublishToDeviceAttributes(payloadJson.toString());
}

int CloudProtocol::PublishToGatewayTelemetry(Json::Value payloadJson)
{
	return PublishToGatewayTelemetry(payloadJson.toString());
}

int CloudProtocol::PublishToGatewayAttributes(Json::Value payloadJson)
{
	return PublishToGatewayAttributes(payloadJson.toString());
}
