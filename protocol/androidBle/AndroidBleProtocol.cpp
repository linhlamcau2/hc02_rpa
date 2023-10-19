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
	if (gateway)
	{
		gateway->localAddActionCallback(bind(&AndroidBleProtocol::OnMessage, this, placeholders::_1, placeholders::_2), "device/androidBle");
		OnAndroidBleProtocolCallbackRegister("bleInfo", bind(&AndroidBleProtocol::OnBleInfo, this, placeholders::_1, placeholders::_2));
		OnAndroidBleProtocolCallbackRegister("newDev", bind(&AndroidBleProtocol::OnNewDevice, this, placeholders::_1, placeholders::_2));
		OnAndroidBleProtocolCallbackRegister("DeviceStatus", bind(&AndroidBleProtocol::OnDeviceStatus, this, placeholders::_1, placeholders::_2));
	}
	else
	{
		LOGE("Must init gateway before init AndroidBleProtocol");
		exit(1);
	}
}

int AndroidBleProtocol::StartScan()
{
	Json::Value dataJson;
	dataJson["cmd"] = "startScanBle";
	dataJson["rqi"] = Util::genRandRQI(16);
	dataJson["data"] = Json::objectValue;
	gateway->LocalPublish("HC/androidBle", dataJson.toString());
	return CODE_OK;
}

int AndroidBleProtocol::StopScan()
{
	Json::Value dataJson;
	dataJson["cmd"] = "stopScanBle";
	dataJson["rqi"] = Util::genRandRQI(16);
	dataJson["data"] = Json::objectValue;
	gateway->LocalPublish("HC/androidBle", dataJson.toString());
	return CODE_OK;
	return CODE_OK;
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
				gateway->LocalPublish("HC/androidBle", respValue.toString());
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						respV["rqi"] = rqi;
						gateway->LocalPublish("HC/androidBle", respV.toString());
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
					if (dataJson.isMember("devKey") && dataJson["devKey"].isString() &&
						dataJson.isMember("vid") && dataJson["vid"].isString() &&
						dataJson.isMember("pid") && dataJson["pid"].isString())
					{
						devKey = dataJson["devKey"].asString();
						pid = dataJson["pid"].asInt();
						vid = dataJson["vid"].asInt();
					}
				}
				Device *device = gateway->AddNewDevice(uuid, Device::ConvertDeviceTypeToName(pid), mac, dataJson, addr, pid, vid, true);
				if (device)
				{
					Json::Value jsonData;
					jsonData["id"] = uuid;
					jsonData["type"] = pid;
					jsonData["data"] = reqValue["data"];
					gateway->AddDeviceToScanList(device);
					bleProtocol->UpdateDeviceKeyDev(addr, devKey);
					// gateway->pushNewDeviceLocal(jsonData);
				}
				else
				{
					respValue["data"]["code"] = CODE_MEMORY_ERROR;
					LOGW("New device id %s err", uuid.c_str());
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
