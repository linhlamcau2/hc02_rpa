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
#include <fstream>

void Gateway::initMqttMessageV2()
{
	OnDeviceRpcCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnControlDevice(Json::Value &reqValue, Json::Value &respValue)
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
				bool rs = device->DoV2(devData);
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
	return 0;
}
