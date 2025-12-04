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

CloudProtocol::CloudProtocol(string mac, string address, int port, string clientId, string username, string password, int keepalive, bool useTls)
	: Mqtt(address, port, clientId, username, password, keepalive, useTls)
{
	this->mac = mac;
	this->useTls = useTls;
	subServerReqTopic = "/v2/server/hc/" + mac + "/json_req";
	subServerRespTopic = "/v2/server/hc/" + mac + "/json_resp";
	subMobileReqTopic = "/v2/mobile/+/hc/" + mac + "/json_req";
	subMobileRespTopic = "/v2/mobile/+/hc/" + mac + "/json_resp";
	pubServerReqTopic = "/v2/hc/" + mac + "/server/json_req";
	pubServerRespTopic = "/v2/hc/" + mac + "/server/json_resp";
	pubMobileReqTopic = "/v2/hc/" + mac + "/mobile/all/json_req";
	pubMobileRespTopic = "/v2/hc/" + mac + "/mobile/";

	subBinRespTopic = "v2/bin/resp/server/" + mac + "/+/+";
	pubBinReqTopic = "v2/bin/req/" + mac + "/server/";

	subSerialRespTopic = "/v2/server/hc/" + mac + "/serial_resp";
	pubSerialReqTopic = "/v2/server/hc/" + mac + "/serial_req";

	hc_request_topic = "/v2/hc/" + mac + "/hc/json_req";

	// willset
	Json::Value jsonValue;
	Json::Value datanValue;
	datanValue["status"] = 0;
	datanValue["version"] = STR(VERSION);
	datanValue["ip"] = Wifi::GetIP();
	jsonValue["cmd"] = "homeController";
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
	addActionCallback(bind(&CloudProtocol::OnMobileReq, this, placeholders::_1, placeholders::_2), subMobileReqTopic);
	addActionCallback(bind(&CloudProtocol::OnServerResp, this, placeholders::_1, placeholders::_2), subServerRespTopic);
	addActionCallback(bind(&CloudProtocol::OnMobileResp, this, placeholders::_1, placeholders::_2), subMobileRespTopic);
	addActionCallback(bind(&CloudProtocol::OnServerBinResp, this, placeholders::_1, placeholders::_2, placeholders::_3), subBinRespTopic);

	addActionCallback(bind(&CloudProtocol::OnServerTestHC, this, placeholders::_1, placeholders::_2), hc_request_topic);
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
				Publish(pubServerRespTopic, respValue.toString());
			}
			else if (rs == CODE_EXIT)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				respValue["rqi"] = rqi;
				Publish(pubServerRespTopic, respValue.toString());
				sleep(2);
				exit(1);
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						respV["rqi"] = rqi;
						Publish(pubServerRespTopic, respV.toString());
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
			SLEEP_MS(1);
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
#ifdef ESP_PLATFORM
	SetLedInternet(true);
#endif
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
			// LOGW("rqi %s not found", rqi.c_str());
			// LOGW("OnServerResp payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnServerResp topic: %s", topic.c_str());
		LOGW("OnServerResp payload: %s", payload.c_str());
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

void CloudProtocol::OnMobileReq(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	Util::LedInternet(false);
	Util::LedServiceLock();
#ifdef ESP_PLATFORM
	SetLedInternet(false);
#endif
	vector<string> topics = Util::splitString(topic, '/');
	if (payloadJson.parse(payload) && payloadJson.isObject() &&
		payloadJson.isMember("cmd") && payloadJson["cmd"].isString() &&
		payloadJson.isMember("rqi") && payloadJson["rqi"].isString() &&
		payloadJson.isMember("data") && payloadJson["data"].isObject())
	{
		string cmd = payloadJson["cmd"].asString();
		string rqi = payloadJson["rqi"].asString();
		if (onRpcCallbackFuncList.find(cmd) != onRpcCallbackFuncList.end())
		{
			OnRpcCallbackFunc onRpmCallbackFunc = onRpcCallbackFuncList[cmd];
			isBusy = true;
			mtx.lock();
			int rs = onRpmCallbackFunc(payloadJson["data"], respValue);
			mtx.unlock();
			isBusy = false;
			if (rs == CODE_OK)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				respValue["rqi"] = rqi;
				LOGD("cloud publish: %s: %s", (pubMobileRespTopic + topics[2] + "/json_resp").c_str(), respValue.toString().c_str());
				Publish(pubMobileRespTopic + topics[2] + "/json_resp", respValue.toString());
			}
			else if (rs == CODE_EXIT)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				respValue["rqi"] = rqi;
				LOGD("cloud publish: %s: %s", (pubMobileRespTopic + topics[2] + "/json_resp").c_str(), respValue.toString().c_str());
				Publish(pubMobileRespTopic + topics[2] + "/json_resp", respValue.toString());
				sleep(2);
				exit(1);
			}
#ifdef __ANDROID__
			else if (rs == CODE_REBOOT)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				respValue["rqi"] = rqi;
				LOGD("cloud publish: %s: %s", (pubMobileRespTopic + topics[2] + "/json_resp").c_str(), respValue.toString().c_str());
				Publish(pubMobileRespTopic + topics[2] + "/json_resp", respValue.toString());
				sleep(2);
				system("su");
				system("reboot");
			}
#endif
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						respV["rqi"] = rqi;
						Publish(pubMobileRespTopic + topics[2] + "/json_resp", respV.toString());
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
			SLEEP_MS(1);
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
#ifdef ESP_PLATFORM
	SetLedInternet(true);
#endif
}
void CloudProtocol::OnMobileResp(string &topic, string &payload)
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
			LOGW("OnServerResp payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnServerResp topic: %s", topic.c_str());
		LOGW("OnServerResp payload: %s", payload.c_str());
	}
	Util::LedInternet(true);
	Util::LedServiceUnlock();
}

void CloudProtocol::OnServerTestHC(std::string &topic, std::string &payload)
{
		std::cout <<"OnServerTestHC"<<std::endl;
    Json::Value payloadJson;

    if (!payloadJson.parse(payload) || !payloadJson.isObject()) {
        std::cout << "Invalid JSON: " << payload << std::endl;
        return;
    }

    if (!payloadJson.isMember("cmd") || !payloadJson["cmd"].isString()) {
        std::cout << "Missing or invalid cmd" << std::endl;
        return;
    }

    if (!payloadJson.isMember("rqi") || !payloadJson["rqi"].isString()) {
        std::cout << "Missing or invalid rqi" << std::endl;
        return;
    }

    std::string cmd = payloadJson["cmd"].asString();
    std::string rqi = payloadJson["rqi"].asString();

    if (cmd != "startTestPCBASmt" && cmd != "startTestPCBATc") {
        std::cout << "Unknown cmd: " << cmd << std::endl;
        return;
    }

    if (!payloadJson.isMember("data") || !payloadJson["data"].isArray()) {
        std::cout << "Missing or invalid data (must be array)" << std::endl;
        return;
    }

    const Json::Value &dataJson = payloadJson["data"];

    std::cout << "CMD OK: " << cmd << std::endl;
    std::cout << "Rqi: " << rqi << std::endl;

    for (const auto &item : dataJson) {
        if (!item.isMember("serial") || !item["serial"].isString() ||
            !item.isMember("version") || !item["version"].isString())
        {
            std::cout << "Invalid item in data array" << std::endl;
            continue;
        }

        std::string serial  = item["serial"].asString();
        std::string version = item["version"].asString();

				active_test_pcba_dhpt(rqi,serial,version);
        std::cout << "Serial: " <<serial<< ", Version: " << version << std::endl;
				break;
    }
}



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
	datanValue["status"] = 1;
	datanValue["version"] = STR(VERSION);
	datanValue["ip"] = Wifi::GetIP();
	jsonValue["cmd"] = "homeController";
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
	sendValue["cmd"] = reqCmd;
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

int CloudProtocol::PublishBinToCloudMessage(string sessionId, int index, char *payload, int payloadLen, string respCmd, Json::Value *respValue, uint32_t timeout)
{
	LOGD("PublishBinToCloudMessage");
	if (!isConnected())
		return CODE_TIMEOUT;
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
		SLEEP_MS(1);
	}
	if (!request.status)
	{
		rs = CODE_ERROR;
	}
	requestList.erase(rqi);
	LOGD("PublishBinToCloudMessage rs: %d", rs);
	return rs;
}

int CloudProtocol::PublishToCloudRecieveBinMessage(string reqCmd, Json::Value &reqValue, string rqi, char *payload, int *payloadLen, uint32_t timeout)
{
	LOGD("PublishToCloudRecieveBinMessage");
	if (!isConnected())
		return CODE_TIMEOUT;
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
	Publish(pubServerReqTopic, sendValue.toString());
	while (!requestBin.status && timeout--)
	{
		SLEEP_MS(1);
	}
	if (!requestBin.status)
	{
		rs = CODE_ERROR;
	}
	requestBinList.erase(rqi);
	LOGD("PublishToCloudRecieveBinMessage rs: %d", rs);
	return rs;
}

bool CloudProtocol::GetTls()
{
	return this->useTls;
}

void CloudProtocol::SetTls(bool useTls)
{
	this->useTls = useTls;
}
