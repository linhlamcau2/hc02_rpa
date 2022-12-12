#ifdef CONFIG_THINGSBOARD

#include "CloudProtocol.h"
#include <string.h>
#include <Log.h>

CloudProtocol::CloudProtocol(string server_address, int server_port, string token, string username, string password, int keepalive) : Mqtt(server_address, server_port, token, username, password, keepalive)
{
}

CloudProtocol::~CloudProtocol()
{
}

void CloudProtocol::init()
{
	addActionCallback(bind(&CloudProtocol::OnDeviceRPC, this, placeholders::_1, placeholders::_2), "v1/devices/me/rpc/request/+");
}

void CloudProtocol::OnDeviceRPC(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	string errs;
	stringstream s(payload);
	Json::CharReaderBuilder b;
	Json::parseFromStream(b, s, &payloadJson, &errs);
	string id = topic.substr(strlen("v1/devices/me/rpc/request/"));
	if (payloadJson.isMember("method") && payloadJson["method"].isString())
	{
		string method = payloadJson["method"].asString();
		if (onRPCCallbackFuncList.find(method) != onRPCCallbackFuncList.end())
		{
			OnRPCCallbackFunc onRPCCallbackFunc = onRPCCallbackFuncList[method];
			int rs = onRPCCallbackFunc(payloadJson, respValue);
			if (rs == 0)
			{
				LOGD("Call %s OK", method.c_str());
				Publish("v1/devices/me/rpc/response/" + id, respValue.toString());
			}
			else
			{
				LOGW("Call %s ERR rs: %d", method.c_str(), rs);
			}
		}
		else
		{
			LOGW("Method %s not registed", method.c_str());
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
	return Publish("v1/devices/me/telemetry", payload);
}

int CloudProtocol::PublishToDeviceAttributes(string payload)
{
	return Publish("v1/devices/me/attributes", payload);
}

int CloudProtocol::PublishToGatewayTelemetry(string payload)
{
	return Publish("v1/gateway/telemetry", payload);
}

int CloudProtocol::PublishToGatewayAttributes(string payload)
{
	return Publish("v1/gateway/attributes", payload);
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

#endif