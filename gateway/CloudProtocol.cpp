#include "CloudProtocol.h"
#include <string.h>
#include <unistd.h>
#include "Log.h"
#include "Util.h"
#include "Wifi.h"

CloudProtocol::CloudProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive) : Mqtt(server_address, server_port, token, username, password, keepalive)
{
	this->mac = mac;
	subTopicV1 = "v1/json/server/" + mac;
	pubTopicV1 = "v1/json/" + mac + "/server";

	subReqTopicV2 = "v2/json/req/server/" + mac;
	subRespTopicV2 = "v2/json/resp/server/" + mac;
	pubReqTopicV2 = "v2/json/req/" + mac + "/server";
	pubRespTopicV2 = "v2/json/resp/" + mac + "/server";
	pubBinReqTopicV2 = "v2/bin/req/" + mac + "/server/";

	Json::Value jsonValue;
	Json::Value datanValue;
	datanValue["STATUS_ID"] = 0;
	datanValue["IP_ADDRESS"] = Wifi::GetIP();
	jsonValue["CMD"] = "HOME_CONTROLLER";
	jsonValue["DATA"] = datanValue;
	SetWillset(pubTopicV1, jsonValue.toString());
}

CloudProtocol::~CloudProtocol()
{
}

void CloudProtocol::init()
{
	Mqtt::init();
	addActionCallback(bind(&CloudProtocol::OnDeviceRpc, this, placeholders::_1, placeholders::_2), subTopicV1);
	addActionCallback(bind(&CloudProtocol::OnDeviceRpcV2, this, placeholders::_1, placeholders::_2), subReqTopicV2);
	addActionCallback(bind(&CloudProtocol::OnServerRespV2, this, placeholders::_1, placeholders::_2), subRespTopicV2);
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
				Publish(pubTopicV1, respValue.toString());
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						Publish(pubTopicV1, respV.toString());
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
				Publish(pubRespTopicV2, respValue.toString());
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						respV["rqi"] = rqi;
						Publish(pubRespTopicV2, respV.toString());
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

void CloudProtocol::OnServerRespV2(string &topic, string &payload)
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
		if (requestList.find(rqi) != requestList.end())
		{
			request_t *request = requestList[rqi];
			if (cmd == request->respCmd)
			{
				request->status = true;
				if (request->respValue && payloadJson.isMember("data") && payloadJson["data"].isObject())
				{
					*request->respValue = payloadJson["data"];
				}
			}
		}
		else
		{
			LOGW("rqi %s not found", rqi.c_str());
			LOGW("OnLocalRespV2 payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnLocalRespV2 topic: %s", topic.c_str());
		LOGW("OnLocalRespV2 payload: %s", payload.c_str());
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
	return Publish(pubTopicV1, jsonValue.toString());
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
	return Publish(pubTopicV1, payload);
}

int CloudProtocol::PublishToDeviceAttributes(string payload)
{
	return Publish(pubTopicV1, payload);
}

int CloudProtocol::PublishToGatewayTelemetry(string payload)
{
	return Publish(pubTopicV1, payload);
}

int CloudProtocol::PublishToGatewayAttributes(string payload)
{
	return Publish(pubTopicV1, payload);
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

int CloudProtocol::PublishToCloudMessageV2(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout)
{
	LOGD("PublishToCloudMessageV2: %s", reqValue.toString().c_str());
	int rs = CODE_OK;
	Json::Value sendValue;
	string rqi = Util::genRandRQI(16);
	sendValue["data"] = reqValue;
	sendValue["rqi"] = rqi;
	sendValue["cmd"] = reqCmd;
	request_t request = {
			.status = false,
			.respCmd = respCmd,
			.respValue = respValue,
	};
	requestList[rqi] = &request;
	Publish(pubReqTopicV2, sendValue.toString());
	while (!request.status && --timeout)
	{
		usleep(1000);
	}
	if (!request.status)
	{
		rs = CODE_ERROR;
	}
	requestList.erase(rqi);
	LOGD("PublishToCloudMessageV2 rs: %d", rs);
	return rs;
}

int CloudProtocol::PublishBinToCloudMessageV2(string sessionId, int index, char *payload, int payloadLen, string respCmd, Json::Value *respValue, uint32_t timeout)
{
	LOGD("PublishBinToCloudMessageV2");
	int rs = CODE_OK;
	request_t request = {
			.status = false,
			.respCmd = respCmd,
			.respValue = respValue,
	};
	string rqi = sessionId + to_string(index);
	requestList[rqi] = &request;
	Publish(pubBinReqTopicV2 + sessionId + "/" + to_string(index), payload, payloadLen);
	while (!request.status && --timeout)
	{
		usleep(1000);
	}
	if (!request.status)
	{
		rs = CODE_ERROR;
	}
	requestList.erase(rqi);
	LOGD("PublishBinToCloudMessageV2 rs: %d", rs);
	return rs;
}
