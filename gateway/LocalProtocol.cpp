#include "LocalProtocol.h"
#include <string.h>
#include <Log.h>
#include "Util.h"

#define HC_CONTROL_TOPIC "HC.CONTROL"
#define HC_RESPONSE_TOPIC "HC.CONTROL.RESPONSE"

LocalProtocol::LocalProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive) : Mqtt(server_address, server_port, token, username, password, keepalive)
{
}

LocalProtocol::~LocalProtocol()
{
}

void LocalProtocol::init()
{
	addActionCallback(bind(&LocalProtocol::OnLocalMessage, this, placeholders::_1, placeholders::_2), HC_CONTROL_TOPIC);
}

int LocalProtocol::LocalConnect()
{
	return Connect();
}

void LocalProtocol::OnConnect(bool isConnected, bool isReconnect)
{
	OnLocalConnect(isConnected, isReconnect);
}

void LocalProtocol::OnLocalMessage(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	string errs;
	stringstream s(payload);
	Json::CharReaderBuilder b;
	Util::LedServiceLock();
	Json::parseFromStream(b, s, &payloadJson, &errs);
	if (payloadJson.isMember("CMD") && payloadJson["CMD"].isString())
	{
		string method = payloadJson["CMD"].asString();
		if (onLocalCallbackFuncList.find(method) != onLocalCallbackFuncList.end())
		{
			OnLocalCallbackFunc onLocalCallbackFunc = onLocalCallbackFuncList[method];
			int rs = onLocalCallbackFunc(payloadJson, respValue);
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
			LOGW("OnLocalMessage payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnLocalMessage topic: %s", topic.c_str());
		LOGW("OnLocalMessage payload: %s", payload.c_str());
	}
	Util::LedServiceUnlock();
}

int LocalProtocol::LocalPublish(string topic, string payload)
{
	return Publish(topic, payload);
}

int LocalProtocol::OnLocalCallbackRegister(string method, OnLocalCallbackFunc onLocalCallbackFunc)
{
	LOGI("OnLocalCallbackRegister method: %s", method.c_str());
	onLocalCallbackFuncList[method] = onLocalCallbackFunc;
	return 0;
}

int LocalProtocol::PublishToLocalMessage(string payload)
{
	return Publish(HC_RESPONSE_TOPIC, payload);
}

int LocalProtocol::PublishToLocalMessage(Json::Value payloadJson)
{
	return PublishToLocalMessage(payloadJson.toString());
}
