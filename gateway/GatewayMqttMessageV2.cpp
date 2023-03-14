#include "Gateway.h"
#include "Log.h"
#include "Db.h"
#include "Util.h"
#include "Ota.h"
#include "BleProtocol.h"
#include "BleDefine.h"
#include "Http.h"
#include "Config.h"
#include "Base64.h"
#include "Wifi.h"
#include <fstream>

void Gateway::initMqttMessageV2()
{
	OnDeviceRpcCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2, placeholders::_3));

	OnLocalCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2, placeholders::_3));
	OnLocalCallbackRegisterV2("getInfoHc", bind(&Gateway::OnGetInfoHC, this, placeholders::_1, placeholders::_2, placeholders::_3));

}

int Gateway::OnControlDevice(Json::Value &reqValue, Json::Value &respValue, string rqi)
{
	LOGD("OnControlDevice");
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("data") && data["data"].isObject())
		{
			string deviceId = data["id"].asString();
			Json::Value devData = data["data"];
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				bool rs = device->Do(devData);
				respValue["data"]["code"] = rs;
			}
			else
			{
				LOGW("Device id %s not found", deviceId.c_str());
				respValue["data"]["code"] = CODE_NOT_FOUND_DEVICE;
			}
		}
		else
		{
			respValue["data"]["code"] = CODE_FORMAT_ERROR;
			LOGW("OnControlDevice %s format error", reqValue.toString().c_str());
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlDevice %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "controlDevRsp";
	respValue["rqi"] = rqi;
	return 0;
}

int Gateway::OnGetInfoHC(Json::Value &reqValue, Json::Value &respValue, string rqi)
{
	LOGD("OnControlDevice");
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		Json::Value jsonValue;
		Json::Value dataValue;
		dataValue["mac"] = mac;
		dataValue["ip"] = Wifi::GetIP();
		dataValue["name"] = "RD_HC";
		dataValue["ver"] = "1.2.9";

		jsonValue["cmd"] = "getInfoHc";
		jsonValue["rqi"] = Util::genRandRQI(16);
		jsonValue["data"] = dataValue;
		LocalProtocol::Publish("HC.CONTROL.RESPONSE.V2", jsonValue.toString());
		respValue["data"]["code"] = CODE_OK;
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlDevice %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "getInfoHcRsp";
	respValue["rqi"] = rqi;
	return 0;
}
