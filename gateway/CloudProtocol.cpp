#include "CloudProtocol.h"
#include <string.h>
#include <Log.h>

#define HC_CONTROL_TOPIC "HC.CONTROL"
#define HC_RESPONSE_TOPIC "HC.CONTROL.RESPONSE"

CloudProtocol::CloudProtocol(string server_address, int server_port, string token, string username, string password, int keepalive) : Mqtt(server_address, server_port, token, username, password, keepalive)
{
}

CloudProtocol::~CloudProtocol()
{
}

void CloudProtocol::init()
{
	addActionCallback(bind(&CloudProtocol::OnDeviceRPC, this, placeholders::_1, placeholders::_2), HC_CONTROL_TOPIC);
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
				Publish(HC_RESPONSE_TOPIC, respValue.toString());
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

int CloudProtocol::OnlineDevice(string deviceName)
{
	Json::Value jsonValue;
	jsonValue["device"] = deviceName;
	return Publish("v1/gateway/connect", jsonValue.toString());
}

int CloudProtocol::OfflineDevice(string deviceName)
{
	Json::Value jsonValue;
	jsonValue["device"] = deviceName;
	return Publish("v1/gateway/disconnect", jsonValue.toString());
}

int CloudProtocol::PublishToDeviceTelemetry(string payload)
{
	return Publish(HC_RESPONSE_TOPIC, payload);
}

int CloudProtocol::PublishToDeviceAttributes(string payload)
{
	return Publish(HC_RESPONSE_TOPIC, payload);
}

int CloudProtocol::PublishToGatewayTelemetry(string payload)
{
	return Publish(HC_RESPONSE_TOPIC, payload);
}

int CloudProtocol::PublishToGatewayAttributes(string payload)
{
	return Publish(HC_RESPONSE_TOPIC, payload);
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
