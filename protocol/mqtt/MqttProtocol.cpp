#include "MqttProtocol.h"
#include "Gateway.h"
#include "DeviceMqtt.h"
#include "Log.h"

MqttProtocol *mqttProtocol = NULL;

MqttProtocol::MqttProtocol()
{
}

MqttProtocol::~MqttProtocol()
{
}

void MqttProtocol::init()
{
	if (gateway)
	{
		gateway->localAddActionCallback(bind(&MqttProtocol::OnMessage, this, placeholders::_1, placeholders::_2), "device/HC");
		OnMqttProtocolCallbackRegister("AddDevice", bind(&MqttProtocol::OnAddDevice, this, placeholders::_1, placeholders::_2));
		OnMqttProtocolCallbackRegister("AddFunction", bind(&MqttProtocol::OnAddFunction, this, placeholders::_1, placeholders::_2));
		OnMqttProtocolCallbackRegister("DeviceStatus", bind(&MqttProtocol::OnDeviceStatus, this, placeholders::_1, placeholders::_2));
	}
	else
	{
		LOGE("Must init gateway before init MqttProtocol");
		exit(1);
	}
}

int MqttProtocol::OnMqttProtocolCallbackRegister(string cmd, OnMqttProtocolCallbackFunc onMqttProtocolCallbackFunc)
{
	LOGI("OnMqttProtocolCallbackRegister cmd: %s", cmd.c_str());
	onMqttProtocolCallbackFuncList[cmd] = onMqttProtocolCallbackFunc;
	return CODE_OK;
}

void MqttProtocol::OnMessage(string &topic, string &payload)
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
		if (onMqttProtocolCallbackFuncList.find(cmd) != onMqttProtocolCallbackFuncList.end())
		{
			OnMqttProtocolCallbackFunc onMqttProtocolCallbackFunc = onMqttProtocolCallbackFuncList[cmd];
			int rs = onMqttProtocolCallbackFunc(payloadJson["data"], respValue);
			if (rs == CODE_OK)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				respValue["rqi"] = rqi;
				gateway->LocalPublish("HC/device", respValue.toString());
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						respV["rqi"] = rqi;
						gateway->LocalPublish("HC/device", respV.toString());
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

int MqttProtocol::SendMessage(string data)
{
	return CODE_OK;
}

int MqttProtocol::OnAddDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnAddDevice");
	respValue["data"]["code"] = CODE_OK;
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
			reqValue.isMember("type") && reqValue["type"].isInt() &&
			reqValue.isMember("mac") && reqValue["mac"].isString())
	{
		string deviceId = reqValue["id"].asString();
		uint32_t type = reqValue["type"].asInt();
		string mac = reqValue["mac"].asString();
		Json::Value dataJson;
		if (reqValue.isMember("data") && reqValue["data"].isObject())
		{
			dataJson = reqValue["data"];
		}
		Device *device = gateway->AddNewDevice(deviceId, mac, mac, dataJson, 0, type, 0, true);
		if (device)
		{
			Json::Value jsonData;
			jsonData["id"] = deviceId;
			jsonData["type"] = type;
			jsonData["data"] = reqValue["data"];
			gateway->AddDeviceToScanList(device);
			gateway->pushNewDeviceLocal(jsonData);
		}
		else
		{
			respValue["data"]["code"] = CODE_MEMORY_ERROR;
			LOGW("New device id %s err", deviceId.c_str());
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnAddDevice %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "OnAddDeviceRsp";
	return CODE_OK;
}

int MqttProtocol::OnAddFunction(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnAddFunction");
	respValue["data"]["code"] = CODE_OK;
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
			reqValue.isMember("data") && reqValue["data"].isObject())
	{
		string deviceId = reqValue["id"].asString();
		Json::Value devData = reqValue["data"];
		Device *device = gateway->getDeviceFromId(deviceId);
		if (device)
		{
			DeviceMqtt *deviceMqtt = dynamic_cast<DeviceMqtt *>(device);
			if (deviceMqtt)
			{
				deviceMqtt->AddFuntion(devData, true);
			}
			else
			{
				respValue["data"]["code"] = CODE_NOT_FOUND_DEVICE;
				LOGW("Device id %s isn't a MQTT device", deviceId.c_str());
			}
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
		LOGW("OnAddFunction %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "AddFunctionRsp";
	return CODE_OK;
}

int MqttProtocol::OnDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
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
