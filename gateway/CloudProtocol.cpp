#include "CloudProtocol.h"
#include <string.h>
#include <Log.h>
#include "Util.h"
#include "Wifi.h"

CloudProtocol::CloudProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive) : Mqtt(server_address, server_port, token, username, password, keepalive)
{
	subTopic = "/v1/server/hc/" + mac + "/json";
	pubTopic = "/v1/hc/" + mac + "/server/json";

	Json::Value jsonValue;
	Json::Value datanValue;
	datanValue["STATUS_ID"] = 0;
	datanValue["IP_ADDRESS"] = Wifi::GetIP();
	jsonValue["CMD"] = "HOME_CONTROLLER";
	jsonValue["DATA"] = datanValue;
	SetWillset(pubTopic, jsonValue.toString());
}

CloudProtocol::~CloudProtocol()
{
}

void CloudProtocol::init()
{
	Mqtt::init();
	addActionCallback(bind(&CloudProtocol::OnDeviceRPC, this, placeholders::_1, placeholders::_2), subTopic);
}

void CloudProtocol::cloudAddActionCallback(ActionCallbackFuncType1 actionCallbackFuncType1, string topic)
{
	addActionCallback(actionCallbackFuncType1, topic);
}

void CloudProtocol::cloudAddActionCallback(ActionCallbackFuncType2 actionCallbackFuncType2, string topic)
{
	addActionCallback(actionCallbackFuncType2, topic);
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
	Json::Reader r;
	r.parse(payload, payloadJson);
	Util::LedInternet(false);
	Util::LedServiceLock();
	if (payloadJson.isObject() && payloadJson.isMember("CMD") && payloadJson["CMD"].isString())
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
	Util::LedInternet(true);
	Util::LedServiceUnlock();
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
	Json::Value datanValue;
	datanValue["STATUS_ID"] = 1;
	datanValue["IP_ADDRESS"] = Wifi::GetIP();
	jsonValue["CMD"] = "HOME_CONTROLLER";
	jsonValue["DATA"] = datanValue;
	return Publish(pubTopic, jsonValue.toString());
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
