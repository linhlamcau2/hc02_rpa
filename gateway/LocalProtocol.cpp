#include "LocalProtocol.h"
#include <string.h>
#include <unistd.h>
#include "Log.h"
#include "Util.h"

#define HC_CONTROL_TOPIC "HC.CONTROL"
#define HC_RESPONSE_TOPIC "HC.CONTROL.RESPONSE"

#ifdef ESP_PLATFORM
LocalProtocol::LocalProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive) : MqttBroker()
#else
LocalProtocol::LocalProtocol(string mac, string server_address, int server_port, string token, string username, string password, int keepalive) : Mqtt(server_address, server_port, token, username, password, keepalive)
#endif
{
	this->mac = mac;
	subReqTopicV2 = "v2/json/req/+/+";
	subRespTopicV2 = "v2/json/resp/+/" + mac;
	pubReqTopicV2 = "v2/json/req/" + mac + "/";
	pubRespTopicV2 = "v2/json/resp/" + mac + "/";
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
	isBusy = false;
	addActionCallback(bind(&LocalProtocol::OnLocalMessage, this, placeholders::_1, placeholders::_2), HC_CONTROL_TOPIC);
	addActionCallback(bind(&LocalProtocol::OnLocalMessageV2, this, placeholders::_1, placeholders::_2), subReqTopicV2);
	// addActionCallback(bind(&LocalProtocol::OnLocalMessageV2, this, placeholders::_1, placeholders::_2), "HC.CONTROL.V2");
	addActionCallback(bind(&LocalProtocol::OnLocalRespV2, this, placeholders::_1, placeholders::_2), subRespTopicV2);
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
	Util::LedServiceLock();
	if (payloadJson.parse(payload) && payloadJson.isObject() &&
			payloadJson.isMember("CMD") && payloadJson["CMD"].isString())
	{
		string cmd = payloadJson["CMD"].asString();
		if (onLocalCallbackFuncList.find(cmd) != onLocalCallbackFuncList.end())
		{
			OnLocalCallbackFunc onLocalCallbackFunc = onLocalCallbackFuncList[cmd];
			isBusy = true;
			int rs = onLocalCallbackFunc(payloadJson, respValue);
			isBusy = false;
			if (rs == CODE_OK)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				Publish(HC_RESPONSE_TOPIC, respValue.toString());
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						Publish(HC_RESPONSE_TOPIC, respV.toString());
					}
				}
			}
			else if (rs == CODE_NOT_RESPONSE)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
			}
			else if (rs == CODE_EXIT)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				Publish(HC_RESPONSE_TOPIC, respValue.toString());
				exit(1);
			}
			else
			{
				LOGW("Call %s ERR rs: %d", cmd.c_str(), rs);
			}
#ifdef ESP_PATFORM
			vTaskDelay(1);
#endif
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
	vector<string> topics = Util::splitString(topic, '/');
	if (topics.size() == 5)
	{
		if (topics[4] == mac || topics[4] == "all")
		{
			Util::LedServiceLock();
			if (payloadJson.parse(payload) && payloadJson.isObject() &&
					payloadJson.isMember("cmd") && payloadJson["cmd"].isString() &&
					payloadJson.isMember("rqi") && payloadJson["rqi"].isString() &&
					payloadJson.isMember("data") && payloadJson["data"].isObject())
			{
				string cmd = payloadJson["cmd"].asString();
				string rqi = payloadJson["rqi"].asString();
				if (onLocalCallbackFuncListV2.find(cmd) != onLocalCallbackFuncListV2.end())
				{
					OnLocalCallbackFunc onLocalCallbackFunc = onLocalCallbackFuncListV2[cmd];
					isBusy = true;
					int rs = onLocalCallbackFunc(payloadJson["data"], respValue);
					isBusy = false;
					if (rs == CODE_OK)
					{
						LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
						respValue["rqi"] = rqi;
						Publish(pubRespTopicV2 + topics[3], respValue.toString());
						// Publish("HC.CONTROL.RESPONSE.V2", respValue.toString());
					}
					else if (rs == CODE_DATA_ARRAY)
					{
						LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
						if (respValue.isArray())
						{
							for (auto &respV : respValue)
							{
								respV["rqi"] = rqi;
								Publish(pubRespTopicV2 + topics[3], respV.toString());
								// Publish("HC.CONTROL.RESPONSE.V2", respValue.toString());
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
#ifdef ESP_PATFORM
					vTaskDelay(1);
#endif
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
		}
	}

	Util::LedServiceUnlock();
}

void LocalProtocol::OnLocalRespV2(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	vector<string> topics = Util::splitString(topic, '/');
	if (topics.size() == 5)
	{
		if (topics[4] == mac || topics[4] == "all")
		{
			Util::LedServiceLock();
			if (payloadJson.parse(payload) && payloadJson.isObject() &&
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
		}
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
	return CODE_OK;
}

int LocalProtocol::OnLocalCallbackRegisterV2(string cmd, OnLocalCallbackFunc onLocalCallbackFunc)
{
	LOGI("OnLocalCallbackRegisterV2 cmd: %s", cmd.c_str());
	onLocalCallbackFuncListV2[cmd] = onLocalCallbackFunc;
	return CODE_OK;
}

int LocalProtocol::PublishToLocalMessage(string &payload)
{
	return Publish(HC_RESPONSE_TOPIC, payload);
}

int LocalProtocol::PublishToLocalMessage(Json::Value &payloadJson)
{
	return Publish(HC_RESPONSE_TOPIC, payloadJson.toString());
}

int LocalProtocol::PublishToLocalMessageV2(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout)
{
	LOGD("PublishToLocalMessageV2: %s", reqValue.toString().c_str());
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
	Publish(pubReqTopicV2 + "all", sendValue.toString());
	while (!request.status && --timeout)
	{
		usleep(1000);
	}
	if (!request.status)
	{
		rs = CODE_ERROR;
	}
	requestList.erase(rqi);
	LOGD("PublishToLocalMessageV2 rs: %d", rs);
	return rs;
}

int LocalProtocol::PublishToLocalMessageV2(string &payload)
{
	return Publish("HC.CONTROL.RESPONSE.V2", payload);
}

int LocalProtocol::PublishToLocalMessageV2(Json::Value &payloadJson)
{
	return Publish("HC.CONTROL.RESPONSE.V2", payloadJson.toString());
}
