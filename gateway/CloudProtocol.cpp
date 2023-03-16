#include "CloudProtocol.h"
#include "Log.h"
#include "Util.h"
#include "Wifi.h"
#include <string.h>

CloudProtocol::CloudProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive) : Mqtt(server_address, server_port, token, username, password, keepalive)
{
	subTopic = "/v1/server/hc/" + mac + "/json";
	pubTopic = "/v1/hc/" + mac + "/server/json";
	subTopicV2 = "/v2/server/hc/" + mac + "/json";
	pubTopicV2 = "/v2/hc/" + mac + "/server/json";

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
	addActionCallback(bind(&CloudProtocol::OnDeviceRpc, this, placeholders::_1, placeholders::_2), subTopic);
	addActionCallback(bind(&CloudProtocol::OnDeviceRpcV2, this, placeholders::_1, placeholders::_2), subTopicV2);
}

void CloudProtocol::cloudAddActionCallback(ActionCallbackFuncType1 actionCallbackFuncType1, string topic)
{
	addActionCallback(actionCallbackFuncType1, topic);
}

void CloudProtocol::cloudAddActionCallback(ActionCallbackFuncType2 actionCallbackFuncType2, string topic)
{
	addActionCallback(actionCallbackFuncType2, topic);
}

void CloudProtocol::cloudAddActionCallback(ActionCallbackFuncType3 actionCallbackFuncType3, string topic)
{
	addActionCallback(actionCallbackFuncType3, topic);
}

void CloudProtocol::cloudAddActionCallback(ActionCallbackFuncType4 actionCallbackFuncType4, string topic)
{
	addActionCallback(actionCallbackFuncType4, topic);
}

int CloudProtocol::CloudConnect()
{
	return Connect();
}

void CloudProtocol::OnConnect(bool isConnected, bool isReconnect)
{
	OnCloudConnect(isConnected, isReconnect);
}

void CloudProtocol::OnDeviceRpc(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	Json::Reader r;
	r.parse(payload, payloadJson);
	Util::LedInternet(false);
	Util::LedServiceLock();
	if (payloadJson.isObject() && payloadJson.isMember("CMD") && payloadJson["CMD"].isString())
	{
		string cmd = payloadJson["CMD"].asString();
		if (onRpcCallbackFuncList.find(cmd) != onRpcCallbackFuncList.end())
		{
			OnRpcCallbackFunc onRpcCallbackFunc = onRpcCallbackFuncList[cmd];
			int rs = onRpcCallbackFunc(payloadJson, respValue);
			if (rs == CODE_OK)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				Publish(pubTopic, respValue.toString());
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						Publish(pubTopic, respV.toString());
					}
				}
			}
			else if (rs == CODE_NOT_RESPONSE)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
			}
			else
			{
				LOGW("Call %s ERR rs: %d", cmd.c_str(), rs);
			}
		}
		else
		{
			LOGW("Method %s not registed", cmd.c_str());
			LOGW("OnDeviceRpc payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnDeviceRpc topic: %s", topic.c_str());
		LOGW("OnDeviceRpc payload: %s", payload.c_str());
	}
	Util::LedInternet(true);
	Util::LedServiceUnlock();
}

void CloudProtocol::OnDeviceRpcV2(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	Json::Reader r;
	r.parse(payload, payloadJson);
	Util::LedInternet(false);
	Util::LedServiceLock();
	if (payloadJson.isObject() &&
			payloadJson.isMember("cmd") && payloadJson["cmd"].isString() &&
			payloadJson.isMember("rqi") && payloadJson["rqi"].isString())
	{
		string cmd = payloadJson["cmd"].asString();
		string rqi = payloadJson["rqi"].asString();
		if (onRpcCallbackFuncListV2.find(cmd) != onRpcCallbackFuncListV2.end())
		{
			OnRpcCallbackFunc onRpcCallbackFunc = onRpcCallbackFuncListV2[cmd];
			int rs = onRpcCallbackFunc(payloadJson, respValue);
			if (rs == CODE_OK)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				respValue["rqi"] = rqi;
				Publish(pubTopicV2, respValue.toString());
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						respV["rqi"] = rqi;
						Publish(pubTopicV2, respV.toString());
					}
				}
			}
			else if (rs == CODE_NOT_RESPONSE)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
			}
			else
			{
				LOGW("Call %s ERR rs: %d", cmd.c_str(), rs);
			}
		}
		else
		{
			LOGW("Cmd %s not registed", cmd.c_str());
			LOGW("OnDeviceRpc payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnDeviceRpc topic: %s", topic.c_str());
		LOGW("OnDeviceRpc payload: %s", payload.c_str());
	}
	Util::LedInternet(true);
	Util::LedServiceUnlock();
}

int CloudProtocol::OnDeviceRpcCallbackRegister(string cmd, OnRpcCallbackFunc onRpcCallbackFunc)
{
	LOGI("OnDeviceRpcCallbackRegister cmd: %s", cmd.c_str());
	onRpcCallbackFuncList[cmd] = onRpcCallbackFunc;
	return CODE_OK;
}

int CloudProtocol::OnDeviceRpcCallbackRegisterV2(string cmd, OnRpcCallbackFunc onRpcCallbackFunc)
{
	LOGI("OnDeviceRpcCallbackRegisterV2 cmd: %s", cmd.c_str());
	onRpcCallbackFuncListV2[cmd] = onRpcCallbackFunc;
	return CODE_OK;
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

int CloudProtocol::CloudPublish(string topic, char *payload, int payloadLen)
{
	LOGD("CloudPublish binary topic: %s", topic.c_str());
	return Publish(topic, payload, payloadLen);
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
