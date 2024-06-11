#include "CloudProtocol.h"
#include "Log.h"
#include "Util.h"
#include "Wifi.h"
#include "Gateway.h"
#include <string.h>
#include <unistd.h>
#include <fstream>

#ifdef ESP_PLATFORM
#include "Led.h"
#include "ButtonSignal.h"
#endif

CloudProtocol::CloudProtocol(string mac, string address, int port, string clientId, string username, string password, int keepalive, char *cert)
#ifdef ESP_PLATFORM
		: Mqtt(address, port, clientId, username, password, keepalive)
#else
		: Mqtt(address, port, clientId, username, password, keepalive, cert)
#endif

{
	this->mac = mac;

	subServerReqTopic = "/dsgw/" + mac + "/command";
	pubServerReqTopic = "/dsgw/" + mac + "/data";

	// willset
	Json::Value jsonValue;
	Json::Value datanValue;
	datanValue["status"] = 0;
	datanValue["version"] = STR(VERSION);
	datanValue["ip"] = Wifi::GetIP();
	jsonValue["type"] = "homeController";
	jsonValue["data"] = datanValue;
	SetWillset(pubServerReqTopic, jsonValue.toString());
}

CloudProtocol::~CloudProtocol()
{
}

void CloudProtocol::init()
{
	Mqtt::init();
	isBusy = false;
	addActionCallback(bind(&CloudProtocol::OnServerReq, this, placeholders::_1, placeholders::_2), subServerReqTopic);
	OnDeviceRpcCallbackRegister("cmd", bind(&CloudProtocol::OnServerCmdReq, this, placeholders::_1, placeholders::_2));
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

void CloudProtocol::OnServerReq(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	Util::LedInternet(false);
	Util::LedServiceLock();
#ifdef ESP_PLATFORM
	SetLedInternet(false);
#endif
	if (payloadJson.parse(payload) && payloadJson.isObject() &&
			payloadJson.isMember("type") && payloadJson["type"].isString() &&
			payloadJson.isMember("time") && payloadJson["time"].isInt() &&
			payloadJson.isMember("data") && payloadJson["data"].isObject())
	{
		string type = payloadJson["type"].asString();
		string rqi = payloadJson["time"].asString();
		if (onRpcCallbackFuncList.find(type) != onRpcCallbackFuncList.end())
		{
			OnRpcCallbackFunc onRpcCallbackFunc = onRpcCallbackFuncList[type];
			isBusy = true;
			int rs = onRpcCallbackFunc(payloadJson["data"], respValue);
			isBusy = false;
			if (rs == CODE_OK)
			{
				LOGD("Call %s OK, rs: %d", type.c_str(), rs);
				respValue["rqi"] = rqi;
				respValue["deviceCode"] = mac;
				respValue["time"] = time(NULL);
				respValue["mac"] = mac;
				respValue["from"] = "GATEWAY";
				respValue["to"] = "CLOUD";
				Publish(pubServerReqTopic, respValue.toString());
			}
			else if (rs == CODE_EXIT)
			{
				LOGD("Call %s OK, rs: %d", type.c_str(), rs);
				respValue["rqi"] = rqi;
				Publish(pubServerReqTopic, respValue.toString());
				sleep(2);
				exit(1);
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", type.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						respV["rqi"] = rqi;
						Publish(pubServerReqTopic, respV.toString());
					}
				}
			}
			else if (rs == CODE_NOT_RESPONSE)
			{
				LOGD("Call %s OK, rs: %d", type.c_str(), rs);
			}
			else
			{
				LOGW("Call %s ERR rs: %d, payload: %s", type.c_str(), rs, payload.c_str());
			}
			SLEEP_MS(1);
		}
		else
		{
			LOGW("Cmd %s not registed", type.c_str());
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
#ifdef ESP_PLATFORM
	SetLedInternet(true);
#endif
}

int CloudProtocol::OnServerCmdReq(Json::Value &dataValue, Json::Value &respValue)
{
	int rs = CODE_ERROR;
	if (dataValue.isMember("command") && dataValue["command"].isString())
	{
		string command = dataValue["command"].asString();
		if (onRpcCmdCallbackFuncList.count(command))
		{
			map<string, OnRpcCallbackFunc> &onRpcCallbackAttributeList = onRpcCmdCallbackFuncList[command];
			if (dataValue.isMember("arguments") && dataValue["arguments"].isObject())
			{
				Json::Value &argumentsValue = dataValue["arguments"];
				if (argumentsValue.isMember("attribute") && argumentsValue["attribute"].isString())
				{
					string attribute = argumentsValue["attribute"].asString();
					if (onRpcCallbackAttributeList.count(attribute))
					{
						OnRpcCallbackFunc onRpcCallbackFunc = onRpcCallbackAttributeList[attribute];
						rs = onRpcCallbackFunc(dataValue, respValue);
						if (rs == CODE_OK)
						{
							LOGD("Call attribute %s OK, rs: %d", attribute.c_str(), rs);
						}
					}
					else
					{
						LOGW("OnServerCmdReq command: %s attribute: %s not registed", command.c_str(), attribute.c_str());
					}
				}
				else
				{
					LOGW("OnServerCmdReq arguments format error");
				}
			}
			else
			{
				LOGW("OnServerCmdReq data format error");
			}
		}
		else
		{
			LOGW("OnServerCmdReq command: %s not registed", command.c_str());
		}
	}
	else
	{
		LOGW("OnServerCmdReq format error");
	}
	return rs;
}

int CloudProtocol::OnDeviceRpcCallbackRegister(string type, OnRpcCallbackFunc onRpcCallbackFunc)
{
	LOGI("OnDeviceRpcCallbackRegister type: %s", type.c_str());
	onRpcCallbackFuncList[type] = onRpcCallbackFunc;
	return CODE_OK;
}

int CloudProtocol::OnDeviceRpcCallbackCmdRegister(string command, string attribute, OnRpcCallbackFunc onRpcCallbackFunc)
{
	LOGI("OnDeviceRpcCallbackCmdRegister command: %s, attribute: %s", command.c_str(), attribute.c_str());
	if (onRpcCmdCallbackFuncList.count(command) == 0)
	{
		map<string, OnRpcCallbackFunc> onRpcCallbackAttributeList;
		onRpcCmdCallbackFuncList[command] = onRpcCallbackAttributeList;
	}
	map<string, OnRpcCallbackFunc> &onRpcCallbackAttributeList = onRpcCmdCallbackFuncList[command];
	onRpcCallbackAttributeList[attribute] = onRpcCallbackFunc;
	return CODE_OK;
}

int CloudProtocol::OnlineHC(string deviceName)
{
	Json::Value jsonValue;
	Json::Value datanValue;
	datanValue["status"] = 1;
	datanValue["version"] = STR(VERSION);
	datanValue["ip"] = Wifi::GetIP();
	jsonValue["type"] = "homeController";
	jsonValue["data"] = datanValue;
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
	return Publish(pubServerReqTopic, payload);
}

int CloudProtocol::CloudPublish(Json::Value payloadJson)
{
	return CloudPublish(payloadJson.toString());
}

int CloudProtocol::PublishToCloudMessage(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout)
{
	LOGD("PublishToCloudMessage: %s", reqValue.toString().c_str());
	if (!isConnected())
	{
		LOGW("error connect to server");
		return CODE_TIMEOUT;
	}
	int rs = CODE_OK;
	Json::Value sendValue;
	string rqi = Util::genRandRQI(16);
	sendValue["data"] = reqValue;
	sendValue["rqi"] = rqi;
	sendValue["type"] = reqCmd;
	request_t request = {
			.status = false,
			.respCmd = respCmd,
			.respValue = respValue,
	};
	requestList[rqi] = &request;
	if (request.pubTopic == "")
	{
		request.pubTopic = pubServerReqTopic;
	}
	LOGD("PublishToCloudMessage: Topic: %s: msg: %s", request.pubTopic.c_str(), (sendValue.toString()).c_str());
	Publish(request.pubTopic, sendValue.toString());
	while (!request.status && timeout--)
	{
		SLEEP_MS(1);
	}
	if (!request.status)
	{
		rs = CODE_ERROR;
	}
	requestList.erase(rqi);
	LOGD("PublishToCloudMessage rs: %d", rs);
	return rs;
}
