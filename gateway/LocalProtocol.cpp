#include "LocalProtocol.h"
#include <string.h>
#include "Log.h"
#include "Util.h"

#define HC_CONTROL_TOPIC "HC.CONTROL"
#define HC_RESPONSE_TOPIC "HC.CONTROL.RESPONSE"
#define HC_CONTROL_TOPIC_V2 "HC.CONTROL.V2"
#define HC_RESPONSE_TOPIC_V2 "HC.CONTROL.RESPONSE.V2"

#ifdef ESP_PLATFORM
LocalProtocol::LocalProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive) : MqttBroker()
#else
LocalProtocol::LocalProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive) : Mqtt(server_address, server_port, token, username, password, keepalive)
#endif
{
}

LocalProtocol::~LocalProtocol()
{
}

void LocalProtocol::init()
{
#ifdef ESP_PLATFORM
	MqttBroker::init();
#else
	Mqtt::init();
#endif
	addActionCallback(bind(&LocalProtocol::OnLocalMessage, this, placeholders::_1, placeholders::_2), HC_CONTROL_TOPIC);
	addActionCallback(bind(&LocalProtocol::OnLocalMessageV2, this, placeholders::_1, placeholders::_2), HC_CONTROL_TOPIC_V2);
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
	Json::Reader r;
	r.parse(payload, payloadJson);
	Util::LedServiceLock();
	if (payloadJson.isObject() && payloadJson.isMember("CMD") && payloadJson["CMD"].isString())
	{
		string cmd = payloadJson["CMD"].asString();
		if (onLocalCallbackFuncList.find(cmd) != onLocalCallbackFuncList.end())
		{
			OnLocalCallbackFunc onLocalCallbackFunc = onLocalCallbackFuncList[cmd];
			int rs = onLocalCallbackFunc(payloadJson, respValue);
			if (rs == 0)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				Publish(HC_RESPONSE_TOPIC, respValue.toString());
			}
			else if (rs == 1)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
			}
			else if (rs == -10)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				Publish(HC_RESPONSE_TOPIC, respValue.toString());
				exit(1);
			}
			else
			{
				LOGW("Call %s ERR rs: %d", cmd.c_str(), rs);
			}
		}
		else
		{
			LOGW("Method %s not registed", cmd.c_str());
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

void LocalProtocol::OnLocalMessageV2(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	Json::Reader r;
	r.parse(payload, payloadJson);
	Util::LedServiceLock();
	if (payloadJson.isObject() &&
			payloadJson.isMember("cmd") && payloadJson["cmd"].isString() &&
			payloadJson.isMember("rqi") && payloadJson["rqi"].isString())
	{
		string cmd = payloadJson["cmd"].asString();
		string rqi = payloadJson["rqi"].asString();
		if (onLocalCallbackFuncListV2.find(cmd) != onLocalCallbackFuncListV2.end())
		{
			OnLocalCallbackFunc onLocalCallbackFunc = onLocalCallbackFuncListV2[cmd];
			int rs = onLocalCallbackFunc(payloadJson, respValue);
			if (rs == 0)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				respValue["rqi"] = rqi;
				Publish(HC_RESPONSE_TOPIC_V2, respValue.toString());
			}
			else if (rs == 1)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
			}
			else if (rs == -10)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				Publish(HC_RESPONSE_TOPIC_V2, respValue.toString());
				exit(1);
			}
			else
			{
				LOGW("Call %s ERR rs: %d", cmd.c_str(), rs);
			}
		}
		else
		{
			LOGW("Method %s not registed", cmd.c_str());
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

int LocalProtocol::OnLocalCallbackRegister(string cmd, OnLocalCallbackFunc onLocalCallbackFunc)
{
	LOGI("OnLocalCallbackRegister cmd: %s", cmd.c_str());
	onLocalCallbackFuncList[cmd] = onLocalCallbackFunc;
	return 0;
}

int LocalProtocol::OnLocalCallbackRegisterV2(string cmd, OnLocalCallbackFunc onLocalCallbackFunc)
{
	LOGI("OnLocalCallbackRegisterV2 cmd: %s", cmd.c_str());
	onLocalCallbackFuncListV2[cmd] = onLocalCallbackFunc;
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
