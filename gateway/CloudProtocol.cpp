#include "CloudProtocol.h"
#include <string.h>
#include <unistd.h>
#include "Log.h"
#include "Util.h"
#include "Wifi.h"

#ifdef ESP_PLATFORM
#include "Led.h"
#include "ButtonSignal.h"
#endif

CloudProtocol::CloudProtocol(string mac, string address, int port, string clientId, string username, string password, int keepalive) : Mqtt(address, port, clientId, username, password, keepalive)
{
	this->mac = mac;

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	subReqTopic = "v2/json/req/server/" + mac;
	subRespTopic = "v2/json/resp/server/" + mac;
	pubReqTopic = "v2/json/req/" + mac + "/server";
	pubRespTopic = "v2/json/resp/" + mac + "/server";

	subBinRespTopic = "v2/bin/resp/server/" + mac + "/+/+";
	pubBinReqTopic = "v2/bin/req/" + mac + "/server/";
#else
	subTopic = "/v1/server/hc/" + mac + "/json";
	pubTopic = "/v1/hc/" + mac + "/server/json";

	// willset
	Json::Value jsonValue;
	Json::Value datanValue;
	datanValue["STATUS_ID"] = 0;
	datanValue["IP_ADDRESS"] = Wifi::GetIP();
	jsonValue["CMD"] = "HOME_CONTROLLER";
	jsonValue["DATA"] = datanValue;
	SetWillset(pubTopic, jsonValue.toString());
#endif
}

CloudProtocol::~CloudProtocol()
{
}

void CloudProtocol::init()
{
	Mqtt::init();
	isBusy = false;
	isConfig = false;
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	addActionCallback(bind(&CloudProtocol::OnServerReq, this, placeholders::_1, placeholders::_2), subReqTopic);
	addActionCallback(bind(&CloudProtocol::OnServerResp, this, placeholders::_1, placeholders::_2), subRespTopic);
	addActionCallback(bind(&CloudProtocol::OnServerBinResp, this, placeholders::_1, placeholders::_2, placeholders::_3), subBinRespTopic);
#else
	addActionCallback(bind(&CloudProtocol::OnDeviceRpc, this, placeholders::_1, placeholders::_2), subTopic);
#endif // CONFIG_USE_MESSAGE_FORMAT_V2
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

void CloudProtocol::SetConfig(bool value)
{
	this->isConfig = value;
}
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
void CloudProtocol::OnServerReq(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	Util::LedInternet(false);
	Util::LedServiceLock();
	if (payloadJson.parse(payload) && payloadJson.isObject() &&
			payloadJson.isMember("cmd") && payloadJson["cmd"].isString() &&
			payloadJson.isMember("rqi") && payloadJson["rqi"].isString() &&
			payloadJson.isMember("data") && payloadJson["data"].isObject())
	{
		string cmd = payloadJson["cmd"].asString();
		string rqi = payloadJson["rqi"].asString();
		if (onRpcCallbackFuncList.find(cmd) != onRpcCallbackFuncList.end())
		{
			OnRpcCallbackFunc onRpcCallbackFunc = onRpcCallbackFuncList[cmd];
			isBusy = true;
			int rs = onRpcCallbackFunc(payloadJson["data"], respValue);
			isBusy = false;
			if (rs == CODE_OK)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				respValue["rqi"] = rqi;
				Publish(pubRespTopic, respValue.toString());
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						respV["rqi"] = rqi;
						Publish(pubRespTopic, respV.toString());
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

void CloudProtocol::OnServerResp(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	Util::LedInternet(false);
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
				if (request->respValue && payloadJson.isMember("data") && payloadJson["data"].isObject())
				{
					*request->respValue = payloadJson["data"];
				}
				request->status = true;
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

void CloudProtocol::OnServerBinResp(string &topic, char *payload, int payloadLen)
{
	vector<string> topics = Util::splitString(topic, '/');
	if (topics.size() != 7)
		return;
	string mac = topics[3];
	string sessionId = topics[5];
	string index = topics[6];
	string rqi = sessionId + index;
	if (requestBinList.find(rqi) != requestBinList.end())
	{
		request_bin_t *requestBin = requestBinList[rqi];
		if (payloadLen <= *requestBin->payloadLen)
		{
			memcpy(requestBin->payload, payload, payloadLen);
			*requestBin->payloadLen = payloadLen;
		}
		else
		{
			memcpy(requestBin->payload, payload, *requestBin->payloadLen);
		}
		requestBin->status = true;
	}
}
#else
void CloudProtocol::OnDeviceRpc(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	Util::LedInternet(false);
	Util::LedServiceLock();
#ifdef ESP_PLATFORM
	bool statusLedInternet = Led::GetLedInternet();
	if (!buttonSignal->GetStatus())
	{
		Led::SetLedInternet(!statusLedInternet);
	}
#endif
	if (payloadJson.parse(payload) && payloadJson.isObject() &&
			payloadJson.isMember("CMD") && payloadJson["CMD"].isString())
	{
		string cmd = payloadJson["CMD"].asString();
		if (onRpcCallbackFuncList.find(cmd) != onRpcCallbackFuncList.end())
		{
			OnRpcCallbackFunc onRpcCallbackFunc = onRpcCallbackFuncList[cmd];
			isBusy = true;
			int rs = onRpcCallbackFunc(payloadJson, respValue);
			isBusy = false;
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
#ifdef ESP_PATFORM
			vTaskDelay(1);
#endif
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
#ifdef ESP_PLATFORM
	if (!buttonSignal->GetStatus())
	{
		Led::SetLedInternet(statusLedInternet);
	}
#endif
	Util::LedInternet(true);
	Util::LedServiceUnlock();
}
#endif

int CloudProtocol::OnDeviceRpcCallbackRegister(string cmd, OnRpcCallbackFunc onRpcCallbackFunc)
{
	LOGI("OnDeviceRpcCallbackRegister cmd: %s", cmd.c_str());
	onRpcCallbackFuncList[cmd] = onRpcCallbackFunc;
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
	return CloudPublish(jsonValue);
}

int CloudProtocol::CloudPublish(string topic, string payload)
{
	return Publish(topic, payload);
}

int CloudProtocol::CloudPublish(string topic, char *payload, int payloadLen)
{
	return Publish(topic, payload, payloadLen);
}

int CloudProtocol::CloudPublish(string payload)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	return Publish(pubReqTopic, payload);
#else
	return Publish(pubTopic, payload);
#endif
}

int CloudProtocol::CloudPublish(Json::Value payloadJson)
{
	return CloudPublish(payloadJson.toString());
}

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
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
	Publish(pubReqTopic, sendValue.toString());
	while (!request.status && timeout--)
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
	Publish(pubBinReqTopic + sessionId + "/" + to_string(index), payload, payloadLen);
	while (!request.status && timeout--)
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

int CloudProtocol::PublishToCloudRecieveBinMessageV2(string reqCmd, Json::Value &reqValue, string rqi, char *payload, int *payloadLen, uint32_t timeout)
{
	LOGD("PublishToCloudRecieveBinMessageV2");
	int rs = CODE_OK;
	request_bin_t requestBin = {
			.status = false,
			.payload = payload,
			.payloadLen = payloadLen,
	};
	requestBinList[rqi] = &requestBin;
	Json::Value sendValue;
	sendValue["data"] = reqValue;
	sendValue["rqi"] = rqi;
	sendValue["cmd"] = reqCmd;
	Publish(pubReqTopic, sendValue.toString());
	while (!requestBin.status && timeout--)
	{
		usleep(1000);
	}
	if (!requestBin.status)
	{
		rs = CODE_ERROR;
	}
	requestBinList.erase(rqi);
	LOGD("PublishToCloudRecieveBinMessageV2 rs: %d", rs);
	return rs;
}
#endif // CONFIG_USE_MESSAGE_FORMAT_V2