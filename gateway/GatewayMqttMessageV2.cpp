#include "Gateway.h"
#include "Log.h"
#include "Db.h"
#include "Util.h"
#include "Ota.h"
#include "BleProtocol.h"
#include "BleDefine.h"
#include "Http.h"
#include "Base64.h"
#include <fstream>

void Gateway::initMqttMessageV2()
{
	OnDeviceRpcCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlGroup", bind(&Gateway::OnControlGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegisterV2("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegisterV2("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlGroup", bind(&Gateway::OnControlGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegisterV2("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
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

int Gateway::OnControlAllDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlAllDevice");
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("data") && data["data"].isObject())
		{
			Json::Value dataValue = data["data"];
			if (dataValue.isObject())
			{
				if (dataValue.isMember(KEY_ATTRIBUTE_ONOFF) && dataValue[KEY_ATTRIBUTE_ONOFF].isInt())
				{
					int value = dataValue[KEY_ATTRIBUTE_ONOFF].asInt();
					bleProtocol->SetOnOffLight(0xFFFF, value, 0, true);
				}
				if (dataValue.isMember(KEY_ATTRIBUTE_DIM) && dataValue[KEY_ATTRIBUTE_DIM].isInt())
				{
					int value = dataValue[KEY_ATTRIBUTE_DIM].asInt();
					uint16_t dim = (value * 65535) / 100;
					bleProtocol->SetDimmingLight(0xFFFF, dim, 0, true);
				}
				if (dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
				{
					int value = dataValue[KEY_ATTRIBUTE_CCT].asInt();
					uint16_t cct = (value * 192) + 800;
					bleProtocol->SetCctLight(0xFFFF, cct, 0, true);
				}
				if (dataValue.isMember(KEY_ATTRIBUTE_HUE) && dataValue[KEY_ATTRIBUTE_HUE].isInt() &&
						dataValue.isMember(KEY_ATTRIBUTE_SATURATION) && dataValue[KEY_ATTRIBUTE_SATURATION].isInt() &&
						dataValue.isMember(KEY_ATTRIBUTE_LUMINANCE) && dataValue[KEY_ATTRIBUTE_LUMINANCE].isInt())
				{
					int h = dataValue[KEY_ATTRIBUTE_HUE].asInt();
					int s = dataValue[KEY_ATTRIBUTE_SATURATION].asInt();
					int l = dataValue[KEY_ATTRIBUTE_LUMINANCE].asInt();
					bleProtocol->SetHSLLight(0xFFFF, h, s, l, 0, true);
				}
				if (dataValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && dataValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
				{
					int value = dataValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
					bleProtocol->CallModeRgb(0xFFFF, value);
				}
				respValue["data"]["code"] = 0;
			}
			else
			{
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
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
	respValue["cmd"] = "controlAllDevRsp";
	return 0;
}

int Gateway::OnControlGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlGroup");
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString() &&
				data.isMember("data") && data["data"].isObject())
		{
			string groupId = data["id"].asString();
			Json::Value devData = data["data"];
			Group *group = getGroupFromId(groupId);
			if (group)
			{
				bool rs = group->DoV2(devData);
				respValue["data"]["code"] = rs;
			}
			else
			{
				LOGW("Group id %s not found", groupId.c_str());
				respValue["data"]["code"] = CODE_NOT_FOUND_GROUP;
			}
		}
		else
		{
			respValue["data"]["code"] = CODE_FORMAT_ERROR;
			LOGW("OnControlGroup %s format error", reqValue.toString().c_str());
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlGroup %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "controlGroupRsp";
	return 0;
}

int Gateway::OnControlScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlScene");
	if (reqValue.isMember("data") && reqValue["data"].isObject())
	{
		Json::Value data = reqValue["data"];
		if (data.isMember("id") && data["id"].isString())
		{
			string sceneId = data["id"].asString();
			SceneBle *scene = getSceneBleFromId(sceneId);
			if (scene)
			{
				bool rs = scene->Do();
				respValue["data"]["code"] = rs;
			}
			else
			{
				LOGW("Scene %s dose not exsit", sceneId.c_str());
				respValue["data"]["code"] = CODE_NOT_FOUND_SCENE;
			}
		}
		else
		{
			respValue["data"]["code"] = CODE_FORMAT_ERROR;
			LOGW("OnControlScene %s format error", reqValue.toString().c_str());
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlScene %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "controlSceneRsp";
	return 0;
}

int Gateway::OnRequestDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRequestDeviceStatus");
	Json::Value deviceData;
	AddAllDeviceStatusV2(deviceData);
	respValue["data"]["code"] = CODE_OK;
	respValue["data"]["device"] = deviceData;
	respValue["cmd"] = "requestDevStt";
	return CODE_OK;
}
