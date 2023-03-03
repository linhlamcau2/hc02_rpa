#include "LocalProtocol.h"
#include <string.h>
#include <Log.h>
#include "Util.h"

#define HC_CONTROL_TOPIC "HC.CONTROL"
#define HC_RESPONSE_TOPIC "HC.CONTROL.RESPONSE"

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
			else if (rs == -10)
			{
				LOGD("Call %s OK, rs: %d", method.c_str(), rs);
				Publish(HC_RESPONSE_TOPIC, respValue.toString());
				exit(1);
			}
			else if (rs == 2)
			{
				if (listMsgPush.size() > 0)
				{
					for (uint32_t i = 0; i < listMsgPush.size(); i++)
					{
						Publish(HC_RESPONSE_TOPIC, listMsgPush[i]);
					}
					listMsgPush.clear();
				}
				else
				{
					LOGW("List msg push empty");
				}
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
