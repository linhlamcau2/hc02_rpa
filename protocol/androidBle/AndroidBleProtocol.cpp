#include "AndroidBleProtocol.h"
#include "Gateway.h"
#include "DeviceMqtt.h"
#include "Log.h"
#include "Util.h"
#include "BleProtocol.h"

AndroidBleProtocol *androidBleProtocol = NULL;

AndroidBleProtocol::AndroidBleProtocol()
{
}

AndroidBleProtocol::~AndroidBleProtocol()
{
}

void AndroidBleProtocol::init()
{
	subTopic = "androidBle/HC";
	pubTopic = "HC/androidBle";
	if (gateway)
	{
		gateway->localAddActionCallback(bind(&AndroidBleProtocol::OnMessage, this, placeholders::_1, placeholders::_2), subTopic);
		gateway->localAddActionCallback(bind(&AndroidBleProtocol::OnAndroidBleResp, this, placeholders::_1, placeholders::_2), subTopic);
		OnAndroidBleProtocolCallbackRegister("bleInfo", bind(&AndroidBleProtocol::OnBleInfo, this, placeholders::_1, placeholders::_2));
		OnAndroidBleProtocolCallbackRegister("newDev", bind(&AndroidBleProtocol::OnNewDevice, this, placeholders::_1, placeholders::_2));
		OnAndroidBleProtocolCallbackRegister("DeviceStatus", bind(&AndroidBleProtocol::OnDeviceStatus, this, placeholders::_1, placeholders::_2));
		OnAndroidBleProtocolCallbackRegister("provisionNormal", bind(&AndroidBleProtocol::OnProvisionNormal, this, placeholders::_1, placeholders::_2));
	}
	else
	{
		LOGE("Must init gateway before init AndroidBleProtocol");
		exit(1);
	}
}

int AndroidBleProtocol::StartScan()
{
	string cmd = "startScanBle";
	Json::Value dataRequest = Json::objectValue;
	Json::Value infoProvision = Json::objectValue;
	infoProvision["netKey"] = gateway->getBleNetKey();
	infoProvision["appKey"] = gateway->getBleAppKey();
	infoProvision["ivIndex"] = gateway->getBleIvIndex();
	infoProvision["addrGw"] = gateway->getBleAddr();
	infoProvision["addProvision"] = gateway->GetNextAndroidProvisionAddr();
	Json::Value dataResponse;
	return PublishToAndroidBleMessage(cmd, dataRequest, cmd, &dataResponse, 2000);
}

int AndroidBleProtocol::StopScan()
{
	string cmd = "stopScanBle";
	Json::Value dataRequest = Json::objectValue;
	Json::Value dataResponse;
	return PublishToAndroidBleMessage(cmd, dataRequest, cmd, &dataResponse, 2000);
}

int AndroidBleProtocol::OnAndroidBleProtocolCallbackRegister(string cmd, OnAndroidBleProtocolCallbackFunc onAndroidBleProtocolCallbackFunc)
{
	LOGI("OnAndroidBleProtocolCallbackRegister cmd: %s", cmd.c_str());
	onAndroidBleProtocolCallbackFuncList[cmd] = onAndroidBleProtocolCallbackFunc;
	return CODE_OK;
}

void AndroidBleProtocol::OnMessage(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	if (payloadJson.parse(payload) && payloadJson.isObject() &&
		payloadJson.isMember("cmd") && payloadJson["cmd"].isString() &&
		payloadJson.isMember("rqi") && payloadJson["rqi"].isString() &&
		payloadJson.isMember("data") && payloadJson["data"].isObject())
	{
		string cmd = payloadJson["cmd"].asString();
		string rqi = payloadJson["rqi"].asString();
		if (onAndroidBleProtocolCallbackFuncList.find(cmd) != onAndroidBleProtocolCallbackFuncList.end())
		{
			OnAndroidBleProtocolCallbackFunc onAndroidBleProtocolCallbackFunc = onAndroidBleProtocolCallbackFuncList[cmd];
			int rs = onAndroidBleProtocolCallbackFunc(payloadJson["data"], respValue);
			if (rs == CODE_OK)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				respValue["rqi"] = rqi;
				gateway->LocalPublish(pubTopic, respValue.toString());
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						respV["rqi"] = rqi;
						gateway->LocalPublish(pubTopic, respV.toString());
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
			LOGW("OnMessage payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnMessage topic: %s", topic.c_str());
		LOGW("OnMessage payload: %s", payload.c_str());
	}
}

int AndroidBleProtocol::SendMessage(string data)
{
	return CODE_OK;
}

int AndroidBleProtocol::OnBleInfo(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("BleInfo Request");
	respValue["cmd"] = "bleInfoRsp";
	respValue["data"]["code"] = CODE_OK;
	respValue["data"]["netKey"] = gateway->getBleNetKey();
	respValue["data"]["appKey"] = gateway->getBleAppKey();
	respValue["data"]["ivIndex"] = gateway->getBleIvIndex();
	respValue["data"]["addrGw"] = gateway->getBleAddr();
	return CODE_OK;
}

int AndroidBleProtocol::OnProvisionNormal(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("Start provision normal");
	int rs = CODE_ERROR;
	respValue["cmd"] = "provisionNormal";
	if (bleProtocol)
	{
		bleProtocol->SetProvisioning(true);
		bleProtocol->StartScan();
		rs = CODE_OK;
	}
	else
	{
		LOGW("BleProtocol null");
	}
	respValue["data"]["code"] = rs;
	return CODE_OK;
}

int AndroidBleProtocol::OnNewDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnNewDevice");
	respValue["data"]["code"] = CODE_OK;
	if (reqValue.isMember("device") && reqValue["device"].isArray())
	{
		Json::Value deviceJsonList = reqValue["device"];
		for (auto deviceJson : deviceJsonList)
		{
			if (deviceJson.isMember("id") && deviceJson["id"].isString() &&
				deviceJson.isMember("mac") && deviceJson["mac"].isString() &&
				deviceJson.isMember("addr") && deviceJson["addr"].isInt())
			{
				string uuid = deviceJson["id"].asString();
				string mac = deviceJson["mac"].asString();
				uint32_t addr = deviceJson["addr"].asInt();
				uint16_t vid;
				uint16_t pid;
				string devKey;
				Json::Value dataJson;
				if (deviceJson.isMember("data") && deviceJson["data"].isObject())
				{
					dataJson = deviceJson["data"];
					if (dataJson.isMember("deviceKey") && dataJson["deviceKey"].isString() &&
						dataJson.isMember("vid") && dataJson["vid"].isInt() &&
						dataJson.isMember("pid") && dataJson["pid"].isInt())
					{
						devKey = dataJson["deviceKey"].asString();
						pid = dataJson["pid"].asInt();
						vid = dataJson["vid"].asInt();

						uint32_t deviceType = Device::ConverPidToDeviveType(pid);
						Device *device = gateway->AddNewDevice(uuid, Device::ConvertDeviceTypeToName(deviceType), mac, dataJson, addr, deviceType, vid, true);
						if (device)
						{
							gateway->AddDeviceToScanList(device);
							bleProtocol->UpdateDeviceKeyDev(addr, devKey);
						}
						else
						{
							respValue["data"]["code"] = CODE_MEMORY_ERROR;
							LOGW("New device id %s err", uuid.c_str());
						}
					}
				}
			}
			else
			{
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
				LOGW("OnDeviceStatus %s format error", reqValue.toString().c_str());
			}
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnNewDevice %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "OnNewDeviceRsp";
	return CODE_OK;
}

int AndroidBleProtocol::OnDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnDeviceStatus");
	respValue["data"]["code"] = CODE_OK;
	if (reqValue.isMember("device") && reqValue["device"].isArray())
	{
		Json::Value deviceJsonList = reqValue["device"];
		for (auto deviceJson : deviceJsonList)
		{
			if (deviceJson.isMember("id") && deviceJson["id"].isString() &&
				deviceJson.isMember("data") && deviceJson["data"].isObject())
			{
				string deviceId = deviceJson["id"].asString();
				Json::Value devData = deviceJson["data"];
				Device *device = gateway->getDeviceFromId(deviceId);
				if (device)
				{
					device->InputData(devData);
				}
				else
				{
					respValue["data"]["code"] = CODE_NOT_FOUND_DEVICE;
					LOGW("Device id %s not found", deviceId.c_str());
				}
			}
			else
			{
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
				LOGW("OnDeviceStatus %s format error", reqValue.toString().c_str());
			}
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnDeviceStatus %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "DeviceStatusRsp";
	return CODE_OK;
}

int AndroidBleProtocol::PublishToAndroidBleMessage(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout)
{
	LOGD("PublishToAndroidBleMessage: %s", reqValue.toString().c_str());
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
		request.pubTopic = pubTopic;
	}
	LOGD("PublishToAndroidBleMessage: Topic: %s: msg: %s", request.pubTopic.c_str(), (sendValue.toString()).c_str());
	gateway->LocalPublish(request.pubTopic, sendValue.toString());
	while (!request.status && timeout--)
	{
		usleep(1000);
	}
	if (!request.status)
	{
		rs = CODE_ERROR;
	}
	requestList.erase(rqi);
	LOGD("PublishToAndroidBleMessage rs: %d", rs);
	return rs;
}

void AndroidBleProtocol::OnAndroidBleResp(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
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
			LOGW("OnAndroidBleResp payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnAndroidBleResp error topic: %s, payload: %s", topic.c_str(), payload.c_str());
	}
}
