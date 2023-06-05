#include <fstream>
#include <algorithm>
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
#ifdef ESP_PLATFORM
#include "Led.h"
#include "Wifi.h"
#endif

void Gateway::initMqttMessage()
{
	OnDeviceRpcCallbackRegister("HC_CONNECT_TO_CLOUD", bind(&Gateway::OnRpcHcConnectCloud, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("HC_BACKUP_DATA", bind(&Gateway::OnRpcHcBackup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("VERSION_HC", bind(&Gateway::OnRpcVersionHc, this, placeholders::_1, placeholders::_2));

	// OnDeviceRpcCallbackRegister("SCAN", bind(&Gateway::OnRpcBleStartScan, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("STOP", bind(&Gateway::OnRpcBleStopScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("RESET_NODE", bind(&Gateway::OnRpcBleDelDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("RESET_BLE", bind(&Gateway::OnRpcBleReset, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("RESET_HC", bind(&Gateway::OnRpcResetFactory, this, placeholders::_1, placeholders::_2));

	// OnDeviceRpcCallbackRegister("CREATE_ROOM", bind(&Gateway::OnRpcCreateRoom, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("ADD_DEVICE_TO_ROOM", bind(&Gateway::OnRpcAddDevToRoom, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("REMOVE_DEVICE_FROM_ROOM", bind(&Gateway::OnRpcRemoveDevFromRoom, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("DELETE_ROOM", bind(&Gateway::OnRpcDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("CHECK_ROOM", bind(&Gateway::OnRpcCheckRoom, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("CREATE_GROUP", bind(&Gateway::OnRpcAddGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DELETE_GROUP", bind(&Gateway::OnRpcDelGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("ADD_DEVICE_TO_GROUP", bind(&Gateway::OnRpcAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DELETE_DEVICE_FROM_GROUP", bind(&Gateway::OnRpcDelDeviceFromGroup, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("CREATE_SCENE", bind(&Gateway::OnRpcAddSceneBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("EDIT_SCENE", bind(&Gateway::OnRpcEditSceneBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DELETE_SCENE", bind(&Gateway::OnRpcDeleteSceneBle, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("CREATE_SCENE_DELAY", bind(&Gateway::OnRpcAddSceneDelay, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("EDIT_SCENE_DELAY", bind(&Gateway::OnRpcEditSceneDelay, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DELETE_SCENE_DELAY", bind(&Gateway::OnRpcDeleteSceneDelay, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("NEW_DEVICE", bind(&Gateway::OnRpcAddTuyaDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DelAllDevice", bind(&Gateway::OnRpcDelAllDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DEVICE", bind(&Gateway::OnRpcControlDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("GROUP", bind(&Gateway::OnRpcControlGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("SCENE", bind(&Gateway::OnRpcControlSceneBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DEVICE_UPDATE", bind(&Gateway::OnRpcUpdateAllTelemetry, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("REMOTE_SSH", bind(&Gateway::OnRpcSSHRemote, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("UPDATE_FIRMWARE", bind(&Gateway::OnRpcUpdateFirmware, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("SCENE_FOR_REMOTE", bind(&Gateway::OnRpcSetSceneForRemote, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DELETE_SCENE_FOR_REMOTE", bind(&Gateway::OnRpcDelSceneForRemote, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("RESET_REMOTE", bind(&Gateway::OnRpcResetRemote, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("SCENE_FOR_SENSOR_LIGHT_PIR", bind(&Gateway::OnRpcScenePirLigtSensor, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("EDIT_SCENE_FOR_SENSOR_LIGHT_PIR", bind(&Gateway::OnRpcEditScenePirLightSensor, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("REMOVE_SCENE_FOR_SENSOR_LIGHT_PIR", bind(&Gateway::OnRpcRemoveScenePirLightSensor, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("SENSOR_UPDATE", bind(&Gateway::OnRpcSensorUpdate, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("SCENE_FOR_SCREEN", bind(&Gateway::OnRpcSceneScreen, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("CREATE_EVENT_TRIGGER", bind(&Gateway::OnRpcAddRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("EDIT_EVENT_TRIGGER", bind(&Gateway::OnRpcEditRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("EVENT_TRIGGER_STATUS", bind(&Gateway::OnRpcSwitchStatusEvent, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DELETE_EVENT_TRIGGER", bind(&Gateway::OnRpcDeleteRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("TAP_TO_RUN", bind(&Gateway::OnRpcTapToRun, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("COUNTDOWN", bind(&Gateway::OnRpcCreateCountDown, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DELETE_COUNTDOWN", bind(&Gateway::OnRpcDelCountDown, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("CREATE_HCL", bind(&Gateway::OnRpcCreateHCL, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("EDIT_HCL", bind(&Gateway::OnRpcEditHCL, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("HCL_RULE_STATUS", bind(&Gateway::OnRpcSwitchStatusEvent, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("STAIRS_SWITCH", bind(&Gateway::OnRpcStairsSwitch, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("EDIT_STAIRS_SWITCH", bind(&Gateway::OnRpcEditStairsSwitch, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DELETE_STAIRS_SWITCH", bind(&Gateway::OnRpcDelStairsSwitch, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("POWER_SWITCH_TIMEOUT", bind(&Gateway::OnRpcPowerSwitchTimeout, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("REMOVE_POWER_SWITCH_TIMEOUT", bind(&Gateway::OnRpcRemovePowerSwitchTimeout, this, placeholders::_1, placeholders::_2));

	OnDeviceRpcCallbackRegister("ADD_DEVICE_SMARTHOME_TO_ROOM", bind(&Gateway::OnRpcAddDeviceSmartHomeToRoom, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("HC_CONNECT_TO_CLOUD", bind(&Gateway::OnRpcHcConnectCloud, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("HC_BACKUP_DATA", bind(&Gateway::OnRpcHcBackup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("VERSION_HC", bind(&Gateway::OnRpcVersionHc, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("SCAN", bind(&Gateway::OnRpcBleStartScan, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("STOP", bind(&Gateway::OnRpcBleStopScan, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("RESET_NODE", bind(&Gateway::OnRpcBleDelDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("RESET_BLE", bind(&Gateway::OnRpcBleReset, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("RESET_HC", bind(&Gateway::OnRpcResetFactory, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("DEVICE_FLASH", bind(&Gateway::OnRpcDeviceFlash, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("CREATE_ROOM", bind(&Gateway::OnRpcCreateRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("ADD_DEVICE_TO_ROOM", bind(&Gateway::OnRpcAddDevToRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("REMOVE_DEVICE_FROM_ROOM", bind(&Gateway::OnRpcRemoveDevFromRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_ROOM", bind(&Gateway::OnRpcDeleteRoom, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("CHECK_ROOM", bind(&Gateway::OnRpcCheckRoom, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_GROUP", bind(&Gateway::OnRpcAddGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_GROUP", bind(&Gateway::OnRpcDelGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("ADD_DEVICE_TO_GROUP", bind(&Gateway::OnRpcAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_DEVICE_FROM_GROUP", bind(&Gateway::OnRpcDelDeviceFromGroup, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_SCENE", bind(&Gateway::OnRpcAddSceneBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EDIT_SCENE", bind(&Gateway::OnRpcEditSceneBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_SCENE", bind(&Gateway::OnRpcDeleteSceneBle, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_SCENE_DELAY", bind(&Gateway::OnRpcAddSceneDelay, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EDIT_SCENE_DELAY", bind(&Gateway::OnRpcEditSceneDelay, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_SCENE_DELAY", bind(&Gateway::OnRpcDeleteSceneDelay, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("NEW_DEVICE", bind(&Gateway::OnRpcAddTuyaDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DelAllDevice", bind(&Gateway::OnRpcDelAllDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DEVICE", bind(&Gateway::OnRpcControlDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("GROUP", bind(&Gateway::OnRpcControlGroup, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("SCENE", bind(&Gateway::OnRpcControlSceneBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DEVICE_UPDATE", bind(&Gateway::OnRpcUpdateAllTelemetry, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("REMOTE_SSH", bind(&Gateway::OnRpcSSHRemote, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("UPDATE_FIRMWARE", bind(&Gateway::OnRpcUpdateFirmware, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("SCENE_FOR_REMOTE", bind(&Gateway::OnRpcSetSceneForRemote, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_SCENE_FOR_REMOTE", bind(&Gateway::OnRpcDelSceneForRemote, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("RESET_REMOTE", bind(&Gateway::OnRpcResetRemote, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("SCENE_FOR_SENSOR_LIGHT_PIR", bind(&Gateway::OnRpcScenePirLigtSensor, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EDIT_SCENE_FOR_SENSOR_LIGHT_PIR", bind(&Gateway::OnRpcEditScenePirLightSensor, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("REMOVE_SCENE_FOR_SENSOR_LIGHT_PIR", bind(&Gateway::OnRpcRemoveScenePirLightSensor, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("SENSOR_UPDATE", bind(&Gateway::OnRpcSensorUpdate, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("SCENE_FOR_SCREEN", bind(&Gateway::OnRpcSceneScreen, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_EVENT_TRIGGER", bind(&Gateway::OnRpcAddRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EDIT_EVENT_TRIGGER", bind(&Gateway::OnRpcEditRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EVENT_TRIGGER_STATUS", bind(&Gateway::OnRpcSwitchStatusEvent, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_EVENT_TRIGGER", bind(&Gateway::OnRpcDeleteRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("TAP_TO_RUN", bind(&Gateway::OnRpcTapToRun, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("COUNTDOWN", bind(&Gateway::OnRpcCreateCountDown, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_COUNTDOWN", bind(&Gateway::OnRpcDelCountDown, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("CREATE_HCL", bind(&Gateway::OnRpcCreateHCL, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EDIT_HCL", bind(&Gateway::OnRpcEditHCL, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("HCL_RULE_STATUS", bind(&Gateway::OnRpcSwitchStatusEvent, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("STAIRS_SWITCH", bind(&Gateway::OnRpcStairsSwitch, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("EDIT_STAIRS_SWITCH", bind(&Gateway::OnRpcEditStairsSwitch, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("DELETE_STAIRS_SWITCH", bind(&Gateway::OnRpcDelStairsSwitch, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("POWER_SWITCH_TIMEOUT", bind(&Gateway::OnRpcPowerSwitchTimeout, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("REMOVE_POWER_SWITCH_TIMEOUT", bind(&Gateway::OnRpcRemovePowerSwitchTimeout, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("ADD_DEVICE_SMARTHOME_TO_ROOM", bind(&Gateway::OnRpcAddDeviceSmartHomeToRoom, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("SET_PASSWD_MQTT_ONLINE", bind(&Gateway::OnRpcSetPwMqttOnline, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnRpcHcConnectCloud(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcHcConnectCloud");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		respValue["CMD"] = "HC_CONNECT_TO_CLOUD";
		if (data.isMember("LATITUDE") && data["LATITUDE"].isDouble() && data.isMember("LONGITUDE") && data["LONGITUDE"].isDouble())
		{
			Util::SetLongitude(data["LONGITUDE"].asDouble());
			Util::SetLatitude(data["LATITUDE"].asDouble());
		}
		if (data.isMember("DORMITORY_ID") && data["DORMITORY_ID"].isString() && data.isMember("REFRESH_TOKEN") && data["REFRESH_TOKEN"].isString())
		{
			string dormitoryId = data["DORMITORY_ID"].asString();
			string refreshToken = data["REFRESH_TOKEN"].asString();
			gateway->setDormitory(dormitoryId);
			gateway->setRefreshToken(refreshToken);
			database->GatewayUpdateDormitory(gateway, dormitoryId);
			database->GatewayUpdateRefreshToken(gateway, refreshToken);
			respValue["DATA"]["SUCCESS"] = true;
		}
		else
		{
			LOGW("Data error");
			respValue["DATA"]["SUCCESS"] = false;
		}
		return CODE_OK;
	}
	else
	{
		LOGW("OnRpcHcConnectCloud %s error", reqValue.toString().c_str());
	}
	return CODE_ERROR;
}

int Gateway::OnRpcHcBackup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcHcBackup");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		string idHc;
		if (data.isMember("HC_ID") && data["HC_ID"].isString())
		{
			idHc = data["HC_ID"].asString();
			gateway->setId(idHc);
			database->GatewayUpdateId(gateway, idHc);
		}

		respValue["CMD"] = "HC_BACKUP_DATA";
		Json::Value dataJsonRsp;
		dataJsonRsp["HC_ID"] = idHc;
		uint8_t status = 0;

		DelAllDevice();
		DelAllGroup();
		DelAllRoom();
		DelAllRule();
		DelAllSceneBle();
		DelAllSceneDelay();

		HTTPRequest *httpRequest = new HTTPRequest();
		httpRequest->setUrl(string(BASE_URL_DEV) + string(RENEW_TOKEN));
		httpRequest->setMethod("POST");

		if (gateway->getDormitory() == "" || gateway->getRefreshToken() == "")
		{
			LOGW("Gateway does not have info dormitory,refresh token");
		}
		string token = httpRequest->GetToken(gateway->getRefreshToken(), gateway->getDormitory());
		if (token != "")
		{
			httpRequest->setToken(token);

			httpRequest->setUrl(string(BASE_URL_DEV) + string(HC_BACKUP_FILE_URL));
			httpRequest->setMethod("POST");
			string resultUpload = httpRequest->UploadFile(gateway->getRefreshToken(), gateway->getDormitory(), DB_NAME);
			LOGD("%s", resultUpload.c_str());
			if (resultUpload != "")
			{
				Json::Value payloadJson;
				if (payloadJson.parse(resultUpload) && payloadJson.isObject() && payloadJson.isMember("url"))
				{
					string urlUploadFile = payloadJson["url"].asString().c_str();
					LOGD("url %s", urlUploadFile.c_str());
					if (httpRequest->CreateBackup(gateway->getRefreshToken(), gateway->getDormitory(), gateway->getMac(), gateway->getVersion(), "", urlUploadFile, idHc))
					{
						status = 1;
					}
				}
				else
				{
					LOGW("url does not available");
					status = 0;
				}
			}
			else
			{
				LOGW("upload file failed");
				status = 0;
			}
		}
		else
		{
			LOGW("Get token failed");
			status = 0;
		}

		delete httpRequest;
		dataJsonRsp["STATUS"] = status;
		respValue["DATA"] = dataJsonRsp;
		return CODE_EXIT;
	}
	else
	{
		LOGW("OnRpcHcBackup %s error", reqValue.toString().c_str());
	}
	return CODE_EXIT;
}

int Gateway::OnRpcVersionHc(Json::Value &reqValue, Json::Value &respValue)
{
	respValue["CMD"] = "VERSION_HC";
	respValue["DATA"]["MAC"] = mac;
	respValue["DATA"]["VERSION_HC"] = STR(VERSION);
	return CODE_OK;
}

int Gateway::OnRpcBleStartScan(Json::Value &reqValue, Json::Value &respValue)
{
	if (bleProtocol)
	{
		bleProtocol->StartScan();
	}
	else
		LOGW("BleProtocol null");
	respValue["code"] = 0;
	return CODE_OK;
}

int Gateway::OnRpcBleStopScan(Json::Value &reqValue, Json::Value &respValue)
{
	if (bleProtocol)
	{
		bleProtocol->StopScan();
	}
	else
		LOGW("BleProtocol null");
	respValue = reqValue;
	return CODE_OK;
}

int Gateway::OnRpcBleReset(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("Reset ble");
	if (bleProtocol)
	{
		bleProtocol->ResetFactory();
	}
	else
		LOGW("BleProtocol null");
	respValue["code"] = 0;
	return CODE_OK;
}

int Gateway::OnRpcResetFactory(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("Reset ble");
	ResetFactory();
	respValue["CMD"] = "RESET_HC";
	Json::Value data;
	data["STATUS"] = "SUCCESS";
	respValue["DATA"] = data;
#ifdef ESP_PLATFORM
	Wifi::WifiStartAP();
#endif
	return CODE_EXIT;
}

// int Gateway::OnRpcBleAddDevice(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("params") && reqValue["params"].isObject())
// 	{
// 		Json::Value dataValue = reqValue["params"];
// 		if ( // dataValue.isMember("id") && dataValue["id"].isString() &&
// 				dataValue.isMember("name") && dataValue["name"].isString() &&
// 				dataValue.isMember("mac") && dataValue["mac"].isString() &&
// 				dataValue.isMember("type") && dataValue["type"].isInt64())
// 		{
// 			string deviceId = ""; // dataValue["id"].asString();
// 			string name = dataValue["name"].asString();
// 			string mac = dataValue["mac"].asString();
// 			uint32_t type = dataValue["type"].asInt64();
// 			int rs = bleProtocol->AddDevice(deviceId, name, mac, type);
// 			respValue["code"] = rs;
// 			return CODE_OK;
// 		}
// 	}
// 	respValue["code"] = -1;
// 	return CODE_ERROR;
// }

int Gateway::OnRpcBleDelDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcBleDelDevice");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isArray())
	{
		respValue["CMD"] = "RESET_NODE";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			string deviceId = dataValue[i].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (device->GetType() == BLE_AC_SCENE_SCREEN_TOUCH)
				{
					numScreenTouchs--;
				}
				if (bleProtocol)
				{
					if (bleProtocol->ResetDev(device->GetAddr()) == CODE_OK)
					{
						dataJsonRsp["SUCCESS"].append(device->GetId());
						database->DeviceInGroupDelDev(device);
						database->DeviceInSceneBleDelDev(device);
						database->DeviceInRoomDelDev(device);
						delDevice(device);
					}
					else
					{
						dataJsonRsp["FAILED"].append(device->GetId());
					}
				}
				else
					LOGW("BleProtocol null");
			}
			else
			{
				LOGD("deviceId %s dose not exist", deviceId.c_str());
			}
		}
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	else
	{
		LOGW("Format error");
	}
	return CODE_ERROR;
}

int Gateway::OnRpcAddRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcAddRule");
	respValue["CMD"] = "EVENT_TRIGGER";
	Json::Value dataJsonRsp = Json::objectValue;
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string eventId = dataValue["EVENT_TRIGGER_ID"].asString();
			dataJsonRsp["EVENT_TRIGGER_ID"] = eventId;
			Rule *rule = AddRule(dataValue, "", true, true);
			if (rule)
			{
				dataJsonRsp["STATUS"] = "SUCCESS";
			}
			else
			{
				dataJsonRsp["STATUS"] = "FAILED";
			}
		}
	}
	respValue["DATA"] = dataJsonRsp;
	return CODE_OK;
}

int Gateway::OnRpcEditRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcEditRule");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "EVENT_TRIGGER";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string eventId = dataValue["EVENT_TRIGGER_ID"].asString();
			dataJsonRsp["EVENT_TRIGGER_ID"] = eventId;
			Rule *rule = getRuleFromId(eventId);
			if (rule)
			{
				rule->DelAllRuleInput();
				rule->DelAllRuleOutput();
				rule = AddRule(dataValue, "", true, true);
				if (rule)
				{
					dataJsonRsp["STATUS"] = "SUCCESS";
				}
				else
				{
					dataJsonRsp["STATUS"] = "FAILED";
				}
			}
			else
			{
				LOGW("rule %s does not exsit", eventId.c_str());
			}
		}
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcSwitchStatusEvent(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcSwitchStatusEvent");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "EVENT_TRIGGER_STATUS";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("STATUS_ID") && dataValue["STATUS_ID"].isInt() && dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string ruleId = dataValue["EVENT_TRIGGER_ID"].asString();
			int status = dataValue["STATUS_ID"].asInt();
			dataJsonRsp["EVENT_TRIGGER_ID"] = ruleId;
			dataJsonRsp["STATUS_ID"] = status;
			Rule *rule = getRuleFromId(ruleId);
			if (rule)
			{
				bool enable = (status) ? true : false;
				if (rule->GetStatus() != enable)
				{
					rule->SetStatus(enable);
					database->RuleUpdateStatus(rule);
				}
			}
			else
			{
				LOGW("Switch rule %s does not exsit", ruleId.c_str());
			}
		}
		else
		{
			LOGW("OnRpcSwitchStatusEvent msg enough info");
		}
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcDeleteRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcDeleteRule");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "DELETE_EVENT_TRIGGER";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string ruleId = dataValue["EVENT_TRIGGER_ID"].asString();
			dataJsonRsp["EVENT_TRIGGER_ID"] = ruleId;
			Rule *rule = gateway->getRuleFromId(ruleId);
			if (rule)
			{
				delRule(rule);
			}
			else
			{
				LOGW("Rule not found");
			}
		}
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcTapToRun(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcTapToRun");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string ruleId = dataValue["EVENT_TRIGGER_ID"].asString();
			Rule *rule = gateway->getRuleFromId(ruleId);
			if (rule)
			{
				rule->RunOutput();
			}
			else
			{
				LOGW("Rule not found");
			}
		}
		return CODE_NOT_RESPONSE;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcCreateHCL(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		respValue["CMD"] = "EVENT_TRIGGER";
		Json::Value dataJsonRsp = Json::objectValue;
		if (data.isMember("EVENT_TRIGGER_ID") && data["EVENT_TRIGGER_ID"].isString() &&
			data.isMember("EACH_DAY") && data["EACH_DAY"].isArray() &&
			data.isMember("GROUP_ID") && data["GROUP_ID"].isString() &&
			data.isMember("STATUS") && data["STATUS"].isInt() &&
			data.isMember("STATES") && data["STATES"].isArray())
		{
			string evevtId = data["EVENT_TRIGGER_ID"].asString();
			dataJsonRsp["EVENT_TRIGGER_ID"] = evevtId;
			string groupId = data["GROUP_ID"].asString();
			Json::Value eachDay = data["EACH_DAY"];
			int status = data["STATUS"].asInt();
			Json::Value states = data["STATES"];
			for (Json::ArrayIndex i = 0; i < states.size(); i++)
			{
				Json::Value state = states[i];
				if (state.isMember("TIME") && state["TIME"].isString() && state.isMember("PROPERTIES") && state["PROPERTIES"].isArray())
				{
					string time = state["TIME"].asString();
					Json::Value properties = state["PROPERTIES"];
					Json::Value dataAddRule;
					dataAddRule["EVENT_TRIGGER_ID"] = evevtId;
					dataAddRule["START_AT"] = time;
					dataAddRule["EACH_DAY"] = eachDay;
					dataAddRule["LOGICAL_OPERATOR_ID"] = -1;
					dataAddRule["STATUS"] = status;
					Json::Value outputGroup;
					outputGroup["GROUP_ID"] = groupId;
					outputGroup["PROPERTIES"] = properties;
					dataAddRule["OUTPUT_GROUPS"] = outputGroup;
					Rule *rule = AddRule(dataAddRule, "", true, true);
					if (rule)
					{
						dataJsonRsp["STATUS"] = "SUCCESS";
					}
					else
					{
						dataJsonRsp["STATUS"] = "FAILED";
					}
				}
			}
		}
		else
		{
			LOGW("Error adding rule");
		}
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcEditHCL(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		respValue["CMD"] = "EDIT_EVENT_TRIGGER";
		Json::Value dataJsonRsp = Json::objectValue;
		if (data.isMember("EVENT_TRIGGER_ID") && data["EVENT_TRIGGER_ID"].isString() &&
			data.isMember("EACH_DAY") && data["EACH_DAY"].isArray() &&
			data.isMember("GROUP_ID") && data["GROUP_ID"].isString() &&
			data.isMember("STATUS") && data["STATUS"].isInt() &&
			data.isMember("STATES") && data["STATES"].isArray())
		{
			string evevtId = data["EVENT_TRIGGER_ID"].asString();
			dataJsonRsp["EVENT_TRIGGER_ID"] = evevtId;
			string groupId = data["GROUP_ID"].asString();
			Json::Value eachDay = data["EACH_DAY"];
			int status = data["STATUS"].asInt();
			Json::Value states = data["STATES"];
			for (Json::ArrayIndex i = 0; i < states.size(); i++)
			{
				Json::Value state = states[i];
				if (state.isMember("TIME") && state["TIME"].isString() && state.isMember("PROPERTIES") && state["PROPERTIES"].isArray())
				{
					string time = state["TIME"].asString();
					Json::Value properties = state["PROPERTIES"];
					Json::Value dataAddRule;
					dataAddRule["EVENT_TRIGGER_ID"] = evevtId;
					dataAddRule["START_AT"] = time;
					dataAddRule["EACH_DAY"] = eachDay;
					dataAddRule["LOGICAL_OPERATOR_ID"] = -1;
					dataAddRule["STATUS"] = status;
					Json::Value outputGroup;
					outputGroup["GROUP_ID"] = groupId;
					outputGroup["PROPERTIES"] = properties;
					dataAddRule["OUTPUT_GROUPS"] = outputGroup;
					Rule *rule = getRuleFromId(evevtId);
					if (rule)
					{
						rule->DelAllRuleInput();
						rule->DelAllRuleOutput();
						rule = AddRule(dataAddRule, "", true, true);
						dataJsonRsp["STATUS"] = "SUCCESS";
					}
					else
					{
						dataJsonRsp["STATUS"] = "FAILED";
					}
				}
			}
		}
		else
		{
			LOGW("Error adding rule");
		}
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcAddSceneBle(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcAddSceneBle");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "CREATE_SCENE";
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["FAILED"] = Json::arrayValue;

		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString() &&
			dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
		{
			string sceneId = dataValue["SCENE_ID"].asString();
			dataJsonRsp["SCENE_ID"] = sceneId;
			int sceneAddr = 1;
			sceneBleListMtx.lock();
			for (const auto &[id, sceneBle] : sceneBleList)
			{
				if (sceneBle->GetAddr() >= sceneAddr)
				{
					sceneAddr = sceneBle->GetAddr() + 1;
				}
			}
			sceneBleListMtx.unlock();
			SceneBle *scene = new SceneBle(sceneId, sceneAddr, sceneId);
			if (scene)
			{
				scene = AddNewSceneBle(scene, true, true);
				if (scene)
				{
					if (dataValue.isMember("ROOM_ID") && dataValue["ROOM_ID"].isString())
					{
						string roomId = dataValue["ROOM_ID"].asString();
						Room *room = getRoomFromId(roomId);
						if (room)
						{
							room->AddSceneBle(scene, true, true);
						}
					}
					Json::Value groupList = dataValue["DEVICES"];
					for (Json::ArrayIndex i = 0; i < groupList.size(); i++)
					{
						Json::Value devInfo = groupList[i];
						if (devInfo.isMember("IDS") && devInfo["IDS"].isArray() && devInfo.isMember("PROPERTIES") && devInfo["PROPERTIES"].isArray())
						{
							Json::Value deviceList = devInfo["IDS"];
							Json::Value deviceProperties = devInfo["PROPERTIES"];

							int modeRgb = 0;
							for (Json::ArrayIndex j = 0; j < deviceProperties.size(); j++)
							{
								Json::Value property = deviceProperties[j];
								if (property.isMember("ID") && property["ID"].isInt() && property.isMember("VALUE") && property["VALUE"].isInt())
								{
									if (property["ID"].asInt() == 23)
									{
										modeRgb = property["VALUE"].asInt();
									}
								}
							}
							for (Json::ArrayIndex j = 0; j < deviceList.size(); j++)
							{
								string deviceId = deviceList[j].asString();
								Device *device = getDeviceFromId(deviceId);
								if (device)
								{
									if (scene->AddDevice(device, deviceProperties, modeRgb, false) == CODE_OK)
									{
										database->DeviceInSceneBleAdd(scene, device, deviceProperties.toString());
										dataJsonRsp["SUCCESS"].append(device->GetId());
									}
									else
									{
										dataJsonRsp["FAILED"].append(device->GetId());
									}
								}
							}
						}
					}
				}
			}
			else
			{
				LOGW("Create new scene error");
			}
		}
		respValue["DATA"] = dataJsonRsp;
	}
	return CODE_OK;
}

int Gateway::OnRpcEditSceneBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString() && dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
		{
			respValue["CMD"] = "EDIT_SCENE";
			Json::Value dataJsonRsp = Json::objectValue;
			dataJsonRsp["FAILED"] = Json::arrayValue;
			string sceneId = dataValue["SCENE_ID"].asString();
			dataJsonRsp["SCENE_ID"] = sceneId;
			SceneBle *scene = getSceneBleFromId(sceneId);
			if (!scene)
			{
				int sceneAddr = 1;
				sceneBleListMtx.lock();
				for (const auto &[id, sceneBle] : sceneBleList)
				{
					if (sceneBle->GetAddr() >= sceneAddr)
					{
						sceneAddr = sceneBle->GetAddr() + 1;
					}
				}
				sceneBleListMtx.unlock();
				scene = new SceneBle(sceneId, sceneAddr, sceneId);
				scene = gateway->AddNewSceneBle(scene, true, true);

				SceneDelay *sceneDelay = getSceneDelayFromId(sceneId);
				if (sceneDelay)
				{
					delSceneDelay(sceneDelay);
				}
			}
			if (scene)
			{
				Json::Value groupList = dataValue["DEVICES"];
				for (Json::ArrayIndex i = 0; i < groupList.size(); i++)
				{
					Json::Value devInfo = groupList[i];
					if (devInfo.isMember("IDS") && devInfo["IDS"].isArray() && devInfo.isMember("PROPERTIES") && devInfo["PROPERTIES"].isArray())
					{
						Json::Value deviceList = devInfo["IDS"];
						Json::Value deviceProperties = devInfo["PROPERTIES"];

						int modeRgb = 0;
						for (Json::ArrayIndex m = 0; m < deviceProperties.size(); m++)
						{
							Json::Value property = deviceProperties[m];
							if (property.isMember("ID") && property["ID"].isInt() && property.isMember("VALUE") && property["VALUE"].isInt())
							{
								if (property["ID"].asInt() == 23)
								{
									modeRgb = property["VALUE"].asInt();
								}
							}
						}
						for (Json::ArrayIndex j = 0; j < deviceList.size(); j++)
						{
							string deviceId = deviceList[j].asString();
							Device *device = getDeviceFromId(deviceId);
							if (device)
							{
								if (scene->AddDevice(device, deviceProperties, modeRgb, false) == CODE_OK)
								{
									database->DeviceInSceneBleAdd(scene, device, deviceProperties.toString());
									dataJsonRsp["SUCCESS"].append(deviceId);
								}
								else
								{
									dataJsonRsp["FAILED"].append(deviceId);
								}
							}
							else
							{
								LOGW("Device %s does not exsit", deviceId.c_str())
							}
						}
					}
				}
			}
			else
			{
				LOGW("Scene %s does not exsit", sceneId.c_str());
			}
			respValue["DATA"] = dataJsonRsp;
		}
	}
	return CODE_OK;
}

int Gateway::OnRpcDeleteSceneBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "DELETE_SCENE";
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["FAILED"] = Json::arrayValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString())
		{
			string sceneId = dataValue["SCENE_ID"].asString();
			dataJsonRsp["SCENE_ID"] = sceneId;
			SceneBle *scene = getSceneBleFromId(sceneId);

			if (scene)
			{
				vector<Device *> listDevInScene;
				int numDevInScene = scene->deviceList.size();
				for (int j = 0; j < numDevInScene; j++)
				{
					listDevInScene.push_back(scene->deviceList[j]->device);
				}
				for (int i = 0; i < numDevInScene; i++)
				{
					if (scene->DelDevice(listDevInScene[i]) == CODE_OK)
					{
						dataJsonRsp["SUCCESS"].append(listDevInScene[i]->GetId());
						database->DeviceInSceneBleDel(scene, listDevInScene[i]);
					}
					else
					{
						dataJsonRsp["FAILED"].append(scene->deviceList[i]->device->GetId());
					}
				}
				delSceneBle(scene);
			}
			else
			{
				LOGW("Scene %s does not exsit", sceneId.c_str());
			}
		}
		respValue["DATA"] = dataJsonRsp;
	}
	return CODE_OK;
}

int Gateway::OnRpcAddSceneDelay(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "CREATE_SCENE_DELAY";
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["FAILED"] = Json::arrayValue;
		dataJsonRsp["SUCCESS"] = Json::arrayValue;

		Json::Value data = reqValue["DATA"];
		string name = "";
		uint32_t addr = 0;
		string id = "";
		int delay = 0;
		if (data.isMember("NAME") && data["NAME"].isString())
		{
			name = data["NAME"].asString();
		}
		if (data.isMember("SCENE_ID") && data["SCENE_ID"].isString())
		{
			id = data["SCENE_ID"].asString();
			dataJsonRsp["SCENE_ID"] = id;
			SceneDelay *sceneDelay = getSceneDelayFromId(id);
			if (!sceneDelay)
			{
				sceneDelay = new SceneDelay(id, addr, name, data);
			}
			if (sceneDelay)
			{
				sceneDelay = AddNewSceneDelay(sceneDelay, true, true, false);
				if (sceneDelay)
				{
					if (data.isMember("DEVICES") && data["DEVICES"].isArray())
					{
						Device *device = NULL;
						for (Json::ArrayIndex i = 0; i < data["DEVICES"].size(); i++)
						{
							Json::Value deviceOutput = data["DEVICES"][i];
							if (deviceOutput.isMember("DELAY") && deviceOutput["DELAY"].isInt())
							{
								delay = deviceOutput["DELAY"].asInt();
							}
							if (deviceOutput.isMember("DEVICE_ID") && deviceOutput["DEVICE_ID"].isString() && deviceOutput.isMember("PROPERTIES") && deviceOutput["PROPERTIES"].isArray())
							{
								string devId = deviceOutput["DEVICE_ID"].asString();
								Json::Value property = deviceOutput["PROPERTIES"];
								device = getDeviceFromId(devId);
								if (device)
								{
									SceneDelayDeviceOutput *sceneDelayDeviceOutput = new SceneDelayDeviceOutput(device, property, delay);
									if (sceneDelayDeviceOutput)
									{
										sceneDelay->AddSceneDelayOutput(sceneDelayDeviceOutput);
										dataJsonRsp["SUCCESS"].append(devId);
									}
									else
									{
										dataJsonRsp["FAILED"].append(devId);
									}
								}
								else
								{
									dataJsonRsp["FAILED"].append(devId);
								}
							}
							else
							{
								LOGW("Data device output error");
							}
						}
					}

					if (data.isMember("GROUPS") && data["GROUPS"].isArray())
					{
						Group *group = NULL;
						for (int j = 0; j < data["GROUPS"].size(); j++)
						{
							Json::Value groupInSceneDelay = data["GROUPS"][j];
							if (groupInSceneDelay.isMember("DELAY") && groupInSceneDelay["DELAY"].isInt())
							{
								delay = groupInSceneDelay["DELAY"].asInt();
							}
							if (groupInSceneDelay.isMember("GROUP_ID") && groupInSceneDelay["GROUP_ID"].isString() && groupInSceneDelay.isMember("PROPERTIES") && groupInSceneDelay["PROPERTIES"].isArray())
							{
								string groupId = groupInSceneDelay["GROUP_ID"].asString();
								Json::Value property = groupInSceneDelay["PROPERTIES"];
								group = getGroupFromId(groupId);
								if (group)
								{
									SceneDelayGroupOutput *sceneDelayGroupOutput = new SceneDelayGroupOutput(group, property, delay);
									if (sceneDelayGroupOutput)
									{
										sceneDelay->AddSceneDelayOutput(sceneDelayGroupOutput);
										dataJsonRsp["SUCCESS"].append(groupId);
									}
									else
									{
										dataJsonRsp["FAILED"].append(groupId);
									}
								}
								else
								{
									dataJsonRsp["FAILED"].append(groupId);
								}
							}
						}
					}
				}
			}
		}
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcEditSceneDelay(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "EDIT_SCENE_DELAY";
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["FAILED"] = Json::arrayValue;
		dataJsonRsp["SUCCESS"] = Json::arrayValue;

		Json::Value data = reqValue["DATA"];
		string name = "";
		uint32_t addr = 0;
		string id = "";
		int delay = 0;
		if (data.isMember("NAME") && data["NAME"].isString())
		{
			name = data["NAME"].asString();
		}
		if (data.isMember("SCENE_ID") && data["SCENE_ID"].isString())
		{
			id = data["SCENE_ID"].asString();
			dataJsonRsp["SCENE_ID"] = id;
			SceneDelay *sceneDelay = getSceneDelayFromId(id);
			if (!sceneDelay)
			{
				sceneDelay = new SceneDelay(id, addr, name, data);
				sceneDelay = AddNewSceneDelay(sceneDelay, true, true, false);
				SceneBle *sceneBle = getSceneBleFromId(id);
				if (sceneBle)
				{
					vector<Device *> listDevInScene;
					int numDevInScene = sceneBle->deviceList.size();
					for (int j = 0; j < numDevInScene; j++)
					{
						listDevInScene.push_back(sceneBle->deviceList[j]->device);
					}
					for (int i = 0; i < numDevInScene; i++)
					{
						sceneBle->DelDevice(listDevInScene[i]);
						database->DeviceInSceneBleDel(sceneBle, listDevInScene[i]);
					}
					delSceneBle(sceneBle);
				}
			}
			if (sceneDelay)
			{
				sceneDelay->DelAllSceneDelayOutput();
				if (data.isMember("DEVICES") && data["DEVICES"].isArray())
				{
					Device *device = NULL;
					for (Json::ArrayIndex i = 0; i < data["DEVICES"].size(); i++)
					{
						Json::Value deviceOutput = data["DEVICES"][i];
						if (deviceOutput.isMember("DELAY") && deviceOutput["DELAY"].isInt())
						{
							delay = deviceOutput["DELAY"].asInt();
						}
						if (deviceOutput.isMember("DEVICE_ID") && deviceOutput["DEVICE_ID"].isString() && deviceOutput.isMember("PROPERTIES") && deviceOutput["PROPERTIES"].isArray())
						{
							string devId = deviceOutput["DEVICE_ID"].asString();
							Json::Value property = deviceOutput["PROPERTIES"];
							device = getDeviceFromId(devId);
							if (device)
							{
								SceneDelayDeviceOutput *sceneDelayDeviceOutput = new SceneDelayDeviceOutput(device, property, delay);
								if (sceneDelayDeviceOutput)
								{
									sceneDelay->AddSceneDelayOutput(sceneDelayDeviceOutput);
									dataJsonRsp["SUCCESS"].append(devId);
								}
								else
								{
									dataJsonRsp["FAILED"].append(devId);
								}
							}
							else
							{
								dataJsonRsp["FAILED"].append(devId);
							}
						}
					}
				}

				if (data.isMember("GROUPS") && data["GROUPS"].isArray())
				{
					Group *group = NULL;
					for (int j = 0; j < data["GROUPS"].size(); j++)
					{
						Json::Value groupInSceneDelay = data["GROUPS"][j];
						if (groupInSceneDelay.isMember("DELAY") && groupInSceneDelay["DELAY"].isInt())
						{
							delay = groupInSceneDelay["DELAY"].asInt();
						}
						if (groupInSceneDelay.isMember("GROUP_ID") && groupInSceneDelay["GROUP_ID"].isString() && groupInSceneDelay.isMember("PROPERTIES") && groupInSceneDelay["PROPERTIES"].isArray())
						{
							string groupId = groupInSceneDelay["GROUP_ID"].asString();
							Json::Value property = groupInSceneDelay["PROPERTIES"];
							group = getGroupFromId(groupId);
							if (group)
							{
								SceneDelayGroupOutput *sceneDelayGroupOutput = new SceneDelayGroupOutput(group, property, delay);
								if (sceneDelayGroupOutput)
								{
									sceneDelay->AddSceneDelayOutput(sceneDelayGroupOutput);
									dataJsonRsp["SUCCESS"].append(groupId);
								}
								else
								{
									dataJsonRsp["FAILED"].append(groupId);
								}
							}
							else
							{
								dataJsonRsp["FAILED"].append(groupId);
							}
						}
					}
				}
			}
		}
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcDeleteSceneDelay(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "DELETE_SCENE_DELAY";
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["FAILED"] = Json::arrayValue;
		dataJsonRsp["SUCCESS"] = Json::arrayValue;

		Json::Value data = reqValue["DATA"];
		if (data.isMember("SCENE_ID") && data["SCENE_ID"].isString())
		{
			string id = data["SCENE_ID"].asString();
			dataJsonRsp["SCENE_ID"] = id;
			SceneDelay *sceneDelay = getSceneDelayFromId(id);
			if (sceneDelay)
			{
				delSceneDelay(sceneDelay);
			}
			else
			{
				LOGW("Scene %s not found", id.c_str());
			}
		}
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcSensorUpdate(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			Device *device = gateway->getDeviceFromId(deviceId);

			if (device)
			{
				if (bleProtocol)
				{
					bleProtocol->UpdateStatusSensorsPm(device->GetAddr());
				}
				else
					LOGW("BleProtocol null");
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
	}
	return CODE_NOT_RESPONSE;
}

static bool status = false;
int Gateway::OnRpcDeviceFlash(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("Device Flash");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		int onoff = 0;
		if (data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString())
		{
			string deviceId = data["DEVICE_ID"].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (status)
				{
					onoff = 1;
					status = false;
				}
				else
				{
					onoff = 0;
					status = true;
				}
				if (bleProtocol)
					bleProtocol->SetOnOffLight(device->GetAddr(), onoff, 5, true);
			}
			else
			{
				LOGW("Device not found");
			}
		}
	}
	return CODE_NOT_RESPONSE;
}

int Gateway::OnRpcCreateRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcCreateRoom %s", reqValue.toString().c_str());
	bool isRoom = false;
	string roomId = "";
	string roomName = "";
	int roomUnicast = 0;
	Room *room = NULL;

	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("NAME") && data["NAME"].isString())
		{
			roomName = data["NAME"].asString();
		}
		if (data.isMember("GROUPS") && data["GROUPS"].isArray() && data.isMember("SCENES") && data["SCENES"].isArray())
		{
			Json::Value groups = data["GROUPS"];
			respValue["CMD"] = "CREATE_ROOM";
			Json::Value jsonDataRsp;
			Json::Value jsonGroupsRsp;
			for (Json::ArrayIndex i = 0; i < groups.size(); i++)
			{
				Json::Value group = groups[i];
				Json::Value jsonGroupRsp;
				if (group.isMember("GROUP_ID") && group["GROUP_ID"].isString() && group.isMember("NAME") && group["NAME"].isString())
				{
					string groupId = group["GROUP_ID"].asString();
					string groupName = group["NAME"].asString();
					jsonGroupRsp["GROUP_ID"] = groupId;
					jsonGroupRsp["FAILED"] = Json::arrayValue;
					int groupAddr = 1;
					groupListMtx.lock();
					for (const auto &[id, groupBle] : groupList)
					{
						if (groupBle->GetAddr() >= groupAddr)
						{
							groupAddr = groupBle->GetAddr() + 1;
						}
					}
					groupListMtx.unlock();
					if (!isRoom)
					{
						isRoom = true;
						roomId = groupId;
						roomUnicast = groupAddr;
						room = getRoomFromId(roomId);
						if (!room)
						{
							room = new Room(roomId, roomUnicast, roomName);
							room = gateway->AddNewRoom(room, true, true);
						}
					}
					Group *newGroup = new Group(groupId, groupAddr, groupName);
					if (newGroup)
					{
						jsonGroupRsp["GROUP_UNICAST_ID"] = groupAddr + 49152;
						if (AddNewGroup(newGroup, true, true))
						{
							if (room)
								room->AddGroup(newGroup, true, true);
							if (group.isMember("DEVICES") && group["DEVICES"].isArray())
							{
								Json::Value devices = group["DEVICES"];
								for (Json::ArrayIndex j = 0; j < devices.size(); j++)
								{
									string deviceId = devices[j].asString();
									Device *device = getDeviceFromId(deviceId);
									if (device)
									{
										if (room)
										{
											room->AddDevice(device, false);
											database->DeviceInRoomAdd(room, device);
										}
										int tempDeviceAddr = device->GetAddr();
										if (newGroup->AddDevice(device, tempDeviceAddr, true) == CODE_OK)
										{
											database->DeviceInGroupAdd(newGroup, device, tempDeviceAddr);
											jsonGroupRsp["SUCCESS"].append(device->GetId());
										}
										else
										{
											jsonGroupRsp["FAILED"].append(device->GetId());
										}
									}
								}
							}
						}
						else
						{
							delete newGroup;
						}
						jsonDataRsp["GROUPS"].append(jsonGroupRsp);
						if (jsonGroupRsp.isMember("SUCCESS"))
							jsonGroupRsp["SUCCESS"].clear();
						if (jsonGroupRsp.isMember("FAILED"))
							jsonGroupRsp["FAILED"].clear();
						jsonGroupRsp["FAILED"] = Json::arrayValue;
					}
				}
			}

			Json::Value scenes = data["SCENES"];
			Json::Value jsonScenesRsp;
			Json::Value jsonSceneRsp;
			for (int i = 0; i < scenes.size(); i++)
			{
				Json::Value scene = scenes[i];
				if (scene.isMember("SCENE_ID") && scene["SCENE_ID"].isString() && scene.isMember("SCENE_NAME") && scene["SCENE_NAME"].isString() && scene.isMember("GROUPS") && scene["GROUPS"].isArray())
				{
					string sceneId = scene["SCENE_ID"].asString();
					string sceneName = scene["SCENE_NAME"].asString();
					jsonSceneRsp["SCENE_ID"] = sceneId;
					jsonSceneRsp["FAILED"] = Json::arrayValue;
					int sceneAddr = 1;
					sceneBleListMtx.lock();
					for (const auto &[id, sceneBle] : sceneBleList)
					{
						if (sceneBle->GetAddr() >= sceneAddr)
						{
							sceneAddr = sceneBle->GetAddr() + 1;
						}
					}
					sceneBleListMtx.unlock();
					SceneBle *sceneInRoom = new SceneBle(sceneId, sceneAddr, sceneName);
					if (sceneInRoom)
					{
						jsonSceneRsp["SCENE_UNICAST_ID"] = sceneAddr;
						sceneInRoom = AddNewSceneBle(sceneInRoom, true, true);
						if (room)
							room->AddSceneBle(sceneInRoom, true, true);
					}

					Json::Value groupsOfScene = scene["GROUPS"];
					for (Json::ArrayIndex j = 0; j < groupsOfScene.size(); j++)
					{
						Json::Value groupOfScene = groupsOfScene[j];
						if (groupOfScene.isMember("GROUP_ID") && groupOfScene["GROUP_ID"].isString() && groupOfScene.isMember("PROPERTIES") && groupOfScene["PROPERTIES"].isArray())
						{
							string idGroup = groupOfScene["GROUP_ID"].asString();
							Json::Value properties = groupOfScene["PROPERTIES"];
							Group *groupInScene = gateway->getGroupFromId(idGroup);
							if (groupInScene)
							{
								groupInScene->Do(properties);
							}
							else
							{
								LOGW("Group %s does not exsit", idGroup.c_str());
							}

							int modeRGB = 0;
							for (Json::ArrayIndex k = 0; k < properties.size(); k++)
							{
								Json::Value property = properties[k];
								if (property.isMember("ID") && property["ID"].isInt() && property.isMember("VALUE") && property["VALUE"].isInt())
								{
									if (property["ID"].asInt() == 23)
									{
										modeRGB = property["VALUE"].asInt();
									}
								}
							}

							if (sceneInRoom)
							{
								for (auto n = 0; n < groupInScene->deviceList.size(); n++)
								{
									string deviceId = groupInScene->deviceList[n]->device->GetId();
									Device *deviceInScene = getDeviceFromId(deviceId);
									if (deviceInScene)
									{
										int tempDeviceAddr = deviceInScene->GetAddr();
										if (sceneInRoom->AddDevice(deviceInScene, properties, modeRGB, false) == CODE_OK)
										{
											database->DeviceInSceneBleAdd(sceneInRoom, deviceInScene, properties.toString());
											jsonSceneRsp["SUCCESS"].append(deviceInScene->GetId());
										}
										else
										{
											jsonSceneRsp["FAILED"].append(deviceInScene->GetId());
										}
									}
								}
							}
						}
					}
					jsonDataRsp["SCENES"].append(jsonSceneRsp);
					if (jsonSceneRsp.isMember("SUCCESS"))
						jsonSceneRsp.removeMember("SUCCESS");
					if (jsonSceneRsp.isMember("FAILED"))
						jsonSceneRsp.removeMember("FAILED");
					jsonSceneRsp["FAILED"] = Json::arrayValue;
				}
			}
			respValue["DATA"] = jsonDataRsp;
			if (room)
			{
				room->SetDataConfig(respValue.toString());
				string ruleStr = respValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->RoomAdd(room);
			}
		}
		else
		{
			LOGW("OnRpcCreateRoom error: %s", respValue.toString().c_str());
		}
	}
	return CODE_OK;
}

int Gateway::OnRpcAddDevToRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcAddDevToRoom %s", reqValue.toString().c_str());
	vector<string> listDevAddGroup;
	map<string, vector<string>> listGroupDevAddRoom;

	bool isRoom = false;
	string roomId = "";
	int roomUnicast = 0;
	Room *room = NULL;
	string roomName = "";

	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("NAME") && data["NAME"].isString())
		{
			roomName = data["NAME"].asString();
		}
		if (data.isMember("GROUPS") && data["GROUPS"].isArray() && data.isMember("SCENES") && data["SCENES"].isArray())
		{
			respValue["CMD"] = "ADD_DEVICE_TO_ROOM";
			Json::Value groupJsonRsp;
			Json::Value sceneJsonRsp;
			Json::Value dataJsonRsp = Json::objectValue;
			groupJsonRsp["FAILED"] = Json::arrayValue;
			sceneJsonRsp["FAILED"] = Json::arrayValue;

			Json::Value groupsAddDev = data["GROUPS"];
			for (Json::ArrayIndex i = 0; i < groupsAddDev.size(); i++)
			{
				Json::Value groupAddDev = groupsAddDev[i];
				if (groupAddDev.isMember("GROUP_ID") && groupAddDev["GROUP_ID"].isString() && groupAddDev.isMember("DEVICES") && groupAddDev["DEVICES"].isArray())
				{
					string groupId = groupAddDev["GROUP_ID"].asString();
					groupJsonRsp["GROUP_ID"] = groupId;
					Json::Value devicesInGroupAddRoom = groupAddDev["DEVICES"];
					Group *groupOfGw = gateway->getGroupFromId(groupId);
					if (groupOfGw)
					{
						if (!isRoom)
						{
							isRoom = true;
							roomId = groupId;
							roomUnicast = groupOfGw->GetAddr();
							room = getRoomFromId(roomId);
							if (!room)
							{
								room = new Room(roomId, roomUnicast, roomName);
								room = gateway->AddNewRoom(room, true, true);
							}
						}
						for (Json::ArrayIndex j = 0; j < devicesInGroupAddRoom.size(); j++)
						{
							string deviceIdGroup = devicesInGroupAddRoom[j].asString();
							Device *deviceAddtoRoom = gateway->getDeviceFromId(deviceIdGroup);
							if (deviceAddtoRoom)
							{
								if (room)
								{
									room->AddDevice(deviceAddtoRoom, false);
									database->DeviceInRoomAdd(room, deviceAddtoRoom);
								}
								int devAddr = deviceAddtoRoom->GetAddr();
								listDevAddGroup.push_back(deviceIdGroup);
								if (groupOfGw->AddDevice(deviceAddtoRoom, devAddr, true) == CODE_OK)
								{
									database->DeviceInGroupAdd(groupOfGw, deviceAddtoRoom, devAddr);
									groupJsonRsp["SUCCESS"].append(deviceIdGroup);
								}
								else
								{
									groupJsonRsp["FAILED"].append(deviceIdGroup);
								}
							}
							else
							{
								LOGW("Device %s does not exsit", deviceIdGroup.c_str());
							}
						}

						listGroupDevAddRoom[groupId] = listDevAddGroup;
						listDevAddGroup.clear();
						dataJsonRsp["GROUPS"].append(groupJsonRsp);
						if (groupJsonRsp.isMember("SUCCESS"))
							groupJsonRsp.removeMember("SUCCESS");
						if (groupJsonRsp.isMember("FAILED"))
							groupJsonRsp.removeMember("FAILED");
						groupJsonRsp["FAILED"] = Json::arrayValue;
					}
					else
					{
						string nameGroup;
						if (groupAddDev.isMember("NAME") && groupAddDev["NAME"].isString())
						{
							nameGroup = groupAddDev["NAME"].asString();
						}
						int groupAddr = 1;
						groupListMtx.lock();
						for (const auto &[id, group] : groupList)
						{
							if (group->GetAddr() >= groupAddr)
							{
								groupAddr = group->GetAddr() + 1;
							}
						}
						groupListMtx.unlock();
						Group *newGroup = new Group(groupId, groupAddr, nameGroup);
						if (newGroup)
						{
							groupJsonRsp["GROUP_UNICAST_ID"] = groupAddr + 49152;
							if (AddNewGroup(newGroup, true, true))
							{
								Json::Value devices = groupAddDev["DEVICES"];
								for (Json::ArrayIndex j = 0; j < devices.size(); j++)
								{
									string deviceIdScene = devices[j].asString();
									Device *device = getDeviceFromId(deviceIdScene);
									if (device)
									{
										if (room)
										{
											room->AddDevice(device, false);
											database->DeviceInRoomAdd(room, device);
										}
										int tempDeviceAddr = device->GetAddr();
										listDevAddGroup.push_back(deviceIdScene);
										if (newGroup->AddDevice(device, tempDeviceAddr, true) == CODE_OK)
										{
											database->DeviceInGroupAdd(newGroup, device, tempDeviceAddr);
											groupJsonRsp["SUCCESS"].append(deviceIdScene);
										}
										else
										{
											groupJsonRsp["FAILED"].append(deviceIdScene);
										}
									}
								}
								listGroupDevAddRoom[groupId] = listDevAddGroup;
								listDevAddGroup.clear();
							}
							else
							{
								delete newGroup;
							}
							dataJsonRsp["GROUPS"].append(groupJsonRsp);
							if (groupJsonRsp.isMember("SUCCESS"))
								groupJsonRsp["SUCCESS"].clear();
							if (groupJsonRsp.isMember("FAILED"))
								groupJsonRsp["FAILED"].clear();
							groupJsonRsp["FAILED"] = Json::arrayValue;
						}
					}
				}
			}

			Json::Value scenesAddDev = data["SCENES"];
			for (Json::ArrayIndex n = 0; n < scenesAddDev.size(); n++)
			{
				Json::Value sceneAddDev = scenesAddDev[n];
				if (sceneAddDev.isMember("SCENE_ID") && sceneAddDev["SCENE_ID"].isString() && sceneAddDev.isMember("GROUPS") && sceneAddDev["GROUPS"].isArray())
				{
					string sceneId = sceneAddDev["SCENE_ID"].asString();
					Json::Value infoDevsAdd = sceneAddDev["GROUPS"];
					sceneJsonRsp["SCENE_ID"] = sceneId;
					SceneBle *sceneOfGw = gateway->getSceneBleFromId(sceneId);
					if (sceneOfGw)
					{
						for (Json::ArrayIndex l = 0; l < infoDevsAdd.size(); l++)
						{
							Json::Value infoDevAdd = infoDevsAdd[l];
							if (infoDevAdd.isMember("GROUP_ID") && infoDevAdd["GROUP_ID"].isString() && infoDevAdd.isMember("PROPERTIES") && infoDevAdd["PROPERTIES"].isArray())
							{
								string groupIdInScene = infoDevAdd["GROUP_ID"].asString();
								Json::Value properties = infoDevAdd["PROPERTIES"];
								Group *groupOfGw = gateway->getGroupFromId(groupIdInScene);
								if (groupOfGw)
								{
									groupOfGw->Do(properties);
								}
								else
								{
									LOGW("Group %s does not exsit", groupIdInScene.c_str());
								}

								int mode = 0;
								for (Json::ArrayIndex m = 0; m < properties.size(); m++)
								{
									Json::Value property = properties[m];
									if (property.isMember("ID") && property["ID"].isInt() && property.isMember("VALUE") && property["VALUE"].isInt())
									{
										if (property["ID"].asInt() == 23)
										{
											mode = property["VALUE"].asInt();
										}
									}
								}
								for (int g = 0; g < listGroupDevAddRoom[groupIdInScene].size(); g++)
								{
									string deviceIdInScene = listGroupDevAddRoom[groupIdInScene][g];
									Device *deviceInScene = getDeviceFromId(deviceIdInScene);
									if (deviceInScene)
									{
										int adrDev = deviceInScene->GetAddr();
										if (sceneOfGw->AddDevice(deviceInScene, properties, mode, false) == CODE_OK)
										{
											database->DeviceInSceneBleAdd(sceneOfGw, deviceInScene, properties.toString());
											sceneJsonRsp["SUCCESS"].append(deviceIdInScene);
										}
										else
										{
											sceneJsonRsp["FAILED"].append(deviceIdInScene);
										}
									}
								}
							}
						}
					}
					else
					{
						LOGW("Scene %s does not exsit", sceneId.c_str());
					}
					dataJsonRsp["SCENES"].append(sceneJsonRsp);
					if (sceneJsonRsp.isMember("SUCCESS"))
						sceneJsonRsp.removeMember("SUCCESS");
					if (sceneJsonRsp.isMember("FAILED"))
						sceneJsonRsp.removeMember("FAILED");
					sceneJsonRsp["FAILED"] = Json::arrayValue;
				}
			}
			respValue["DATA"] = dataJsonRsp;
			if (room)
			{
				room->SetDataConfig(respValue.toString());
				string ruleStr = respValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->RoomAdd(room);
			}
		}
		else
		{
			LOGW("OnRpcAddDevToRoom error: %s", respValue.toString().c_str());
		}
	}
	return CODE_OK;
}

int Gateway::OnRpcRemoveDevFromRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcRemoveDevFromRoom: %s", reqValue.toString().c_str());
	bool isRoom = false;
	string roomId = "";
	int roomUnicast = 0;
	Room *room = NULL;

	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("GROUPS") && data["GROUPS"].isArray() && data.isMember("SCENES") && data["SCENES"].isArray())
		{
			respValue["CMD"] = "REMOVE_DEVICE_FROM_ROOM";
			Json::Value groupsDelRoom = data["GROUPS"];
			Json::Value scenesDelRoom = data["SCENES"];
			Json::Value dataJsonRsp = Json::objectValue;
			Json::Value groupJsonRsp;
			Json::Value sceneJsonRsp;
			groupJsonRsp["FAILED"] = Json::arrayValue;
			sceneJsonRsp["FAILED"] = Json::arrayValue;

			for (Json::ArrayIndex i = 0; i < groupsDelRoom.size(); i++)
			{
				Json::Value groupDelRoom = groupsDelRoom[i];
				if (groupDelRoom.isMember("GROUP_ID") && groupDelRoom["GROUP_ID"].isString())
				{
					string groupId = groupDelRoom["GROUP_ID"].asString();
					groupJsonRsp["GROUP_ID"] = groupId;
					Group *groupOfGw = getGroupFromId(groupId);
					if (groupOfGw)
					{
						if (!isRoom)
						{
							isRoom = true;
							roomId = groupId;
							roomUnicast = groupOfGw->GetAddr();
							room = getRoomFromId(roomId);
							if (!room)
							{
								room = new Room(roomId, roomUnicast, "");
								room = gateway->AddNewRoom(room, true, true);
								// database->RoomAdd(room);
							}
						}
						if (groupDelRoom.isMember("DEVICES") && groupDelRoom["DEVICES"].isArray())
						{
							Json::Value devicesDelGroup = groupDelRoom["DEVICES"];
							for (Json::ArrayIndex j = 0; j < devicesDelGroup.size(); j++)
							{
								string deviceId = devicesDelGroup[j].asString();
								Device *deviceDelGroup = getDeviceFromId(deviceId);
								if (deviceDelGroup)
								{
									if (room)
									{
										room->DelDevice(deviceDelGroup);
										database->DeviceInRoomDel(room, deviceDelGroup);
									}
									int adrDev = deviceDelGroup->GetAddr();
									if (groupOfGw->DelDevice(deviceDelGroup, adrDev) == CODE_OK)
									{
										database->DeviceInGroupDel(groupOfGw, deviceDelGroup, adrDev);
										groupJsonRsp["SUCCESS"].append(deviceId);
									}
									else
									{
										groupJsonRsp["FAILED"].append(deviceId);
									}
								}
								else
								{
									LOGW("Device %s does not exsit", deviceId.c_str());
								}
							}
						}
					}
					else
					{
						LOGW("Group %s does not exsit", groupId.c_str());
					}
				}
				dataJsonRsp["GROUPS"].append(groupJsonRsp);
				if (groupJsonRsp.isMember("SUCCESS"))
					groupJsonRsp.removeMember("SUCCESS");
				if (groupJsonRsp.isMember("FAILED"))
					groupJsonRsp.removeMember("FAILED");
				groupJsonRsp["FAILED"] = Json::arrayValue;
			}

			for (Json::ArrayIndex n = 0; n < scenesDelRoom.size(); n++)
			{
				Json::Value sceneDelRoom = scenesDelRoom[n];
				if (sceneDelRoom.isMember("SCENE_ID") && sceneDelRoom["SCENE_ID"].isString())
				{
					string sceneId = sceneDelRoom["SCENE_ID"].asString();
					sceneJsonRsp["SCENE_ID"] = sceneId;
					SceneBle *sceneOfGw = getSceneBleFromId(sceneId);
					if (sceneOfGw)
					{
						if (sceneDelRoom.isMember("DEVICES") && sceneDelRoom["DEVICES"].isArray())
						{
							Json::Value devicesDelScene = sceneDelRoom["DEVICES"];
							for (Json::ArrayIndex m = 0; m < devicesDelScene.size(); m++)
							{
								string deviceIdDelScene = devicesDelScene[m].asString();
								Device *deviceDelScene = getDeviceFromId(deviceIdDelScene);
								if (deviceDelScene)
								{
									if (sceneOfGw->DelDevice(deviceDelScene) == CODE_OK)
									{
										database->DeviceInSceneBleDel(sceneOfGw, deviceDelScene);
										sceneJsonRsp["SUCCESS"].append(deviceIdDelScene);
									}
									else
									{
										sceneJsonRsp["FAILED"].append(deviceIdDelScene);
									}
								}
								else
								{
									LOGW("Device %s does not exsit", deviceIdDelScene.c_str());
								}
							}
						}
					}
					else
					{
						LOGW("Scene %s does not exsit", sceneId.c_str());
					}
				}
				dataJsonRsp["SCENES"].append(sceneJsonRsp);
				if (sceneJsonRsp.isMember("SUCCESS"))
					sceneJsonRsp.removeMember("SUCCESS");
				if (sceneJsonRsp.isMember("FAILED"))
					sceneJsonRsp.removeMember("FAILED");
				sceneJsonRsp["FAILED"] = Json::arrayValue;
			}
			respValue["DATA"] = dataJsonRsp;
			if (room)
			{
				room->SetDataConfig(respValue.toString());
				string ruleStr = respValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->RoomAdd(room);
			}
		}
		else
		{
			LOGW("OnRpcRemoveDevFromRoom msg error");
		}
	}
	return CODE_OK;
}

int Gateway::OnRpcDeleteRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcDeleteRoom: %s", reqValue.toString().c_str());
	bool isRoom = false;
	string roomId = "";
	int roomUnicast = 0;
	Room *room = NULL;
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("GROUPS") && data["GROUPS"].isArray() && data.isMember("SCENES") && data["SCENES"].isArray())
		{
			respValue["CMD"] = "DELETE_ROOM";
			Json::Value groupsDelRoom = data["GROUPS"];
			Json::Value scenesDelRoom = data["SCENES"];
			Json::Value dataJsonRsp = Json::objectValue;
			Json::Value groupJsonRsp;
			Json::Value sceneJsonRsp;
			groupJsonRsp["FAILED"] = Json::arrayValue;
			sceneJsonRsp["FAILED"] = Json::arrayValue;

			bool hasDeviceDelGroupFailed;
			bool hasDeviceDelSceneFailed;

			for (Json::ArrayIndex i = 0; i < groupsDelRoom.size(); i++)
			{
				string groupId = groupsDelRoom[i].asString();
				groupJsonRsp["GROUP_ID"] = groupId;
				Group *groupOfGw = getGroupFromId(groupId);
				if (groupOfGw)
				{
					if (!isRoom)
					{
						isRoom = true;
						roomId = groupId;
						roomUnicast = groupOfGw->GetAddr();
						room = getRoomFromId(roomId);
						if (!room)
						{
							room = new Room(roomId, roomUnicast, "");
							room = gateway->AddNewRoom(room, true, true);
						}
					}
					hasDeviceDelGroupFailed = false;
					int numDeviceInGroup = groupOfGw->deviceList.size();
					vector<DeviceInGroup *> listGr = groupOfGw->deviceList;
					for (int j = 0; j < numDeviceInGroup; j++)
					{
						if (room)
						{
							room->DelDevice(listGr[j]->device);
							database->DeviceInRoomDel(room, listGr[j]->device);
						}
						if (groupOfGw->DelDevice(listGr[j]->device, listGr[j]->device->GetAddr()) == CODE_OK)
						{
							database->DeviceInGroupDel(groupOfGw, listGr[j]->device, listGr[j]->device->GetAddr());
							groupJsonRsp["SUCCESS"].append(listGr[j]->device->GetId());
						}
						else
						{
							hasDeviceDelGroupFailed = true;
							groupJsonRsp["FAILED"].append(listGr[j]->device->GetId());
						}
					}
					if (!hasDeviceDelGroupFailed)
					{
						delGroup(groupOfGw);
					}
				}
				else
				{
					LOGW("Group %s does not exsit", groupId.c_str());
				}
				dataJsonRsp["GROUPS"].append(groupJsonRsp);
				if (groupJsonRsp.isMember("SUCCESS"))
					groupJsonRsp.removeMember("SUCCESS");
				if (groupJsonRsp.isMember("FAILED"))
					groupJsonRsp.removeMember("FAILED");
				groupJsonRsp["FAILED"] = Json::arrayValue;
			}

			for (Json::ArrayIndex n = 0; n < scenesDelRoom.size(); n++)
			{
				string sceneId = scenesDelRoom[n].asString();
				sceneJsonRsp["SCENE_ID"] = sceneId;
				SceneBle *sceneOfGw = getSceneBleFromId(sceneId);
				if (sceneOfGw)
				{
					hasDeviceDelSceneFailed = false;
					int numDevInScene = sceneOfGw->deviceList.size();
					vector<DeviceInSceneBle *> listScensBle = sceneOfGw->deviceList;
					for (int m = 0; m < numDevInScene; m++)
					{
						if (sceneOfGw->DelDevice(listScensBle[m]->device) == CODE_OK)
						{
							database->DeviceInSceneBleDel(sceneOfGw, listScensBle[m]->device);
							sceneJsonRsp["SUCCESS"].append(listScensBle[m]->device->GetId());
						}
						else
						{
							hasDeviceDelSceneFailed = true;
							sceneJsonRsp["FAILED"].append(listScensBle[m]->device->GetId());
						}
					}
					if (!hasDeviceDelSceneFailed)
					{
						delSceneBle(sceneOfGw);
					}
				}
				else
				{
					LOGW("Scene %s does not exsit", sceneId.c_str());
				}
				dataJsonRsp["SCENES"].append(sceneJsonRsp);
				if (sceneJsonRsp.isMember("SUCCESS"))
					sceneJsonRsp.removeMember("SUCCESS");
				if (sceneJsonRsp.isMember("FAILED"))
					sceneJsonRsp.removeMember("FAILED");
				sceneJsonRsp["FAILED"] = Json::arrayValue;
			}
			respValue["DATA"] = dataJsonRsp;
			if (room)
			{
				room->SetDataConfig(respValue.toString());
				string ruleStr = respValue.toString();
				ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
				database->RoomAdd(room);
			}
		}
	}
	else
	{
		LOGW("OnRpcDeleteRoom msg error");
	}
	return CODE_OK;
}

int Gateway::OnRpcCheckRoom(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("Check room")
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("ROOM_ID") && data["ROOM_ID"].isString())
		{
			string roomId = data["ROOM_ID"].asString();
			if (roomId != "")
			{
				Room *room = getRoomFromId(roomId);
				if (room)
				{
					string dataConfig = room->GetDataConfig();
					Json::Value dataJson;
					if (dataJson.parse(dataConfig) && dataJson.isObject())
					{
						respValue = dataJson;
						return CODE_OK;
					}
					else
					{
						LOGW("dataConfig is invalid: %s", dataConfig.c_str());
					}
				}
				else
				{
					LOGW("Room does not exist");
				}
			}
		}
	}
	return CODE_ERROR;
}

int Gateway::OnRpcAddGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcAddGroup %s", reqValue.toString().c_str());
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
			dataValue.isMember("NAME") && dataValue["NAME"].isString())
		{
			string groupId = dataValue["GROUP_ID"].asString();
			string groupName = dataValue["NAME"].asString();
			int groupAddr = 1;
			groupListMtx.lock();
			for (const auto &[id, group] : groupList)
			{
				if (group->GetAddr() >= groupAddr)
				{
					groupAddr = group->GetAddr() + 1;
				}
			}
			groupListMtx.unlock();
			Group *group = new Group(groupId, groupAddr, groupName);
			if (group)
			{
				if (AddNewGroup(group, true, true))
				{
					if (dataValue.isMember("ROOM_ID") && dataValue["ROOM_ID"].isString())
					{
						string roomId = dataValue["ROOM_ID"].asString();
						Room *room = getRoomFromId(roomId);
						if (room)
						{
							room->AddGroup(group, true, true);
						}
					}
					respValue["CMD"] = "CREATE_GROUP";
					Json::Value data;
					data["GROUP_ID"] = groupId;
					if (dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
					{
						Json::Value devices = dataValue["DEVICES"];
						for (Json::ArrayIndex i = 0; i < devices.size(); i++)
						{
							string deviceId = devices[i].asString();
							Device *device = getDeviceFromId(deviceId);
							if (device)
							{
								int tempDeviceAddr = device->GetAddr();
								if (group->AddDevice(device, tempDeviceAddr, true) == CODE_OK)
								{
									database->DeviceInGroupAdd(group, device, tempDeviceAddr);
									data["SUCCESS"].append(device->GetId());
									// respValue["code"] = 0;
									// return CODE_OK;
								}
								else
								{
									data["FAILED"].append(device->GetId());
								}
							}
						}
						// respValue["code"] = 0;
						// return CODE_OK;
						respValue["DATA"] = data;
					}
				}
				else
				{
					delete group;
				}
			}
		}
	}
	return CODE_OK;
}

int Gateway::OnRpcUpdateGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcUpdateGroup");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("id") && dataValue["id"].isInt() &&
			dataValue.isMember("name") && dataValue["name"].isString())
		{
			int addr = dataValue["id"].asInt();
			string name = dataValue["name"].asString();
			Group *group = getGroupFromAddr(addr);
			if (group)
			{
				group->SetName(name);
				database->GroupUpdate(group);
				respValue["code"] = 0;
				return CODE_OK;
			}
		}
	}
	respValue["code"] = -1;
	return CODE_ERROR;
}

int Gateway::OnRpcDelGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcDelGroup");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString())
		{
			respValue["CMD"] = "DELETE_GROUP";
			Json::Value data;
			string groupId = dataValue["GROUP_ID"].asString();
			data["GROUP_ID"] = groupId;
			Group *group = getGroupFromId(groupId);
			if (group)
			{
				int temp_groupUnicastId = group->GetAddr();
				bool hasDeviceDelGroupFailed = false;
				int numberDevOfGroup = group->deviceList.size();
				vector<Device *> tempDevInGroup;
				for (int i = 0; i < numberDevOfGroup; i++)
				{
					tempDevInGroup.push_back(group->deviceList[i]->device);
				}
				for (int j = 0; j < numberDevOfGroup; j++)
				{
					if (group->DelDevice(tempDevInGroup[j], tempDevInGroup[j]->GetAddr()) == CODE_OK)
					{
						database->DeviceInGroupDel(group, tempDevInGroup[j], tempDevInGroup[j]->GetAddr());
						data["SUCCESS"].append(tempDevInGroup[j]->GetId());
					}
					else
					{
						hasDeviceDelGroupFailed = true;
						data["FAILED"].append(tempDevInGroup[j]->GetId());
					}
				}
				if (!hasDeviceDelGroupFailed)
				{
					delGroup(group);
				}
			}
			respValue["DATA"] = data;
		}
	}
	return CODE_OK;
}

int Gateway::OnRpcAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcAddDeviceToGroup");
	try
	{
		if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
		{
			Json::Value dataValue = reqValue["DATA"];
			if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
				dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
			{
				respValue["CMD"] = "ADD_DEVICE_TO_GROUP";
				Json::Value data;
				string groupId = dataValue["GROUP_ID"].asString();
				data["GROUP_ID"] = groupId;
				Json::Value deviceList = dataValue["DEVICES"];
				Group *group = getGroupFromId(groupId);
				if (group)
				{
					for (int i = 0; i < (int)deviceList.size(); i++)
					{
						string deviceId = deviceList[i].asString();
						Device *device = getDeviceFromId(deviceId);
						if (device)
						{
							int tempDeviceAddr = device->GetAddr();
							if (group->AddDevice(device, tempDeviceAddr, true) == CODE_OK)
							{
								database->DeviceInGroupAdd(group, device, tempDeviceAddr);
								data["SUCCESS"].append(device->GetId());
								// respValue["code"] = 0;
								// return CODE_OK;
							}
							else
							{
								data["FAILED"].append(device->GetId());
							}
						}
					}
				}
				else
				{
					LOGW("Group id: %s does not exist", groupId.c_str());
				}
				respValue["DATA"] = data;
			}
		}
		// respValue["code"] = -1;
		// return CODE_ERROR;
	}
	catch (const char *msg)
	{
		LOGE("OnRpcAddDeviceToGroup fail");
		return CODE_ERROR;
	}
	return CODE_OK;
}

/**
 * @brief delete device from group
 *
 * @param [in] reqValue json input
 * @param [out] respValue json output
 * @return int -1 - error, 0 - success
 */
int Gateway::OnRpcDelDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcDelDeviceFromGroup");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
			dataValue.isMember("DEVICES") && dataValue["DEVICES"].isArray())
		{
			respValue["CMD"] = "DELETE_DEVICE_FROM_GROUP";
			Json::Value data;
			string groupId = dataValue["GROUP_ID"].asString();
			data["GROUP_ID"] = groupId;
			Group *group = getGroupFromId(groupId);
			if (group)
			{
				Json::Value deviceList = dataValue["DEVICES"];
				for (int i = 0; i < (int)deviceList.size(); i++)
				{
					string deviceId = deviceList[i].asString();
					Device *device = getDeviceFromId(deviceId);
					if (device)
					{
						int tempDeviceAddr = device->GetAddr();
						if (group->DelDevice(device, tempDeviceAddr) == CODE_OK)
						{
							database->DeviceInGroupDel(group, device, tempDeviceAddr);
							data["SUCCESS"].append(device->GetId());
							// respValue["code"] = 0;
							// return CODE_OK;
						}
						else
						{
							data["FAILED"].append(device->GetId());
						}
					}
				}
			}
			else
			{
				LOGW("Group id: %s does not exist", groupId.c_str());
			}
			respValue["DATA"] = data;
		}
	}
	// respValue["code"] = -1;
	return CODE_OK;
}

static int GetIdButton(string button)
{
	string listButtonId[] = {"BUTTON_1", "BUTTON_2", "BUTTON_3", "BUTTON_4", "BUTTON_5", "BUTTON_6"};
	for (int i = 0; i < 6; i++)
	{
		if (listButtonId[i].compare(button) == 0)
		{
			return (i + 1);
		}
	}
	return CODE_ERROR;
}

int Gateway::OnRpcSetSceneForRemote(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "SCENE_FOR_REMOTE";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() && dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString() && dataValue.isMember("BUTTON_VALUE") && dataValue["BUTTON_VALUE"].isString() && dataValue.isMember("MODE_VALUE") && dataValue["MODE_VALUE"].isInt())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			string buttonValue = dataValue["BUTTON_VALUE"].asString();
			int buttonId = GetIdButton(buttonValue);
			string sceneId = dataValue["SCENE_ID"].asString();
			int modeValue = dataValue["MODE_VALUE"].asInt();
			dataJsonRsp["DEVICE_ID"] = deviceId;
			dataJsonRsp["BUTTON_VALUE"] = buttonValue;
			dataJsonRsp["MODE_VALUE"] = modeValue;
			dataJsonRsp["SCENE_ID"] = sceneId;
			respValue["DATA"] = dataJsonRsp;
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				SceneBle *scene = getSceneBleFromId(sceneId);
				if (scene)
				{
					if (bleProtocol)
					{
						if (device->GetType() == BLE_DC_SCENE_CONTACT || device->GetType() == BLE_REMOTE_M3 || device->GetType() == BLE_REMOTE_M3_V2 || device->GetType() == BLE_REMOTE_M4)
						{
							if (bleProtocol->SetSceneSwitchSceneDC(device->GetAddr(), buttonId, modeValue, scene->GetAddr(), 0) == 0)
							{
								return CODE_OK;
							}
						}
						else if (device->GetType() == BLE_AC_SCENE_CONTACT || device->GetType() == BLE_AC_SCENE_CONTACT_RGB || device->GetType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE)
						{
							if (bleProtocol->SetSceneSwitchSceneAC(device->GetAddr(), buttonId, modeValue, scene->GetAddr(), 0) == 0)
							{
								return CODE_OK;
							}
						}
						else
						{
							LOGW("Type dev don't support function");
						}
					}
					else
						LOGW("BleProtocol null");
				}
				else
				{
					LOGW("Scene %s does not exsit", sceneId.c_str())
				}
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
		else
		{
			LOGW("Data error");
		}
	}
	return CODE_ERROR;
}

int Gateway::OnRpcDelSceneForRemote(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "DELETE_SCENE_FOR_REMOTE";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() && dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString() && dataValue.isMember("BUTTON_VALUE") && dataValue["BUTTON_VALUE"].isString() && dataValue.isMember("MODE_VALUE") && dataValue["MODE_VALUE"].isInt())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			string buttonValue = dataValue["BUTTON_VALUE"].asString();
			int buttonId = GetIdButton(buttonValue);
			string sceneId = dataValue["SCENE_ID"].asString();
			int modeValue = dataValue["MODE_VALUE"].asInt();
			dataJsonRsp["DEVICE_ID"] = deviceId;
			dataJsonRsp["BUTTON_VALUE"] = buttonValue;
			dataJsonRsp["MODE_VALUE"] = modeValue;
			dataJsonRsp["SCENE_ID"] = sceneId;
			respValue["DATA"] = dataJsonRsp;
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (bleProtocol)
				{
					if (device->GetType() == BLE_DC_SCENE_CONTACT)
					{
						if (bleProtocol->DelSceneSwitchSceneDC(device->GetAddr(), buttonId, modeValue) == 0)
						{
							return CODE_OK;
						}
					}
					else if (device->GetType() == BLE_AC_SCENE_CONTACT || device->GetType() == BLE_AC_SCENE_CONTACT_RGB || device->GetType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE)
					{
						if (bleProtocol->DelSceneSwitchSceneAC(device->GetAddr(), buttonId, modeValue) == 0)
						{
							return CODE_OK;
						}
					}
				}
				else
					LOGW("BleProtocol null");
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
	}
	return CODE_ERROR;
}

int Gateway::OnRpcResetRemote(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "RESET_REMOTE";
		Json::Value dataJsonRsp = Json::objectValue;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			dataJsonRsp["DEVICE_ID"] = deviceId;
			respValue["DATA"] = dataJsonRsp;
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (bleProtocol)
				{
					if (device->GetType() == BLE_DC_SCENE_CONTACT)
					{
						for (int i = 1; i <= 6; i++)
						{
							if (bleProtocol->DelSceneSwitchSceneDC(device->GetAddr(), i, 0))
							{
								LOGW("del scene error button %d, mode 0", i);
							}
							if (bleProtocol->DelSceneSwitchSceneDC(device->GetAddr(), i, 1))
							{
								LOGW("del scene error button %d, mode 1", i);
							}
						}
					}
					else if (device->GetType() == BLE_AC_SCENE_CONTACT || device->GetType() == BLE_AC_SCENE_CONTACT_RGB || device->GetType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE)
					{
						for (int j = 1; j <= 6; j++)
						{
							if (bleProtocol->DelSceneSwitchSceneAC(device->GetAddr(), j, 0))
							{
								LOGW("del scene error button %d, mode 0", j);
							}
							if (bleProtocol->DelSceneSwitchSceneAC(device->GetAddr(), j, 1))
							{
								LOGW("del scene error button %d, mode 1", j);
							}
						}
					}
				}
				else
					LOGW("BleProtocol null");
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
	}
	return CODE_OK;
}

int Gateway::OnRpcScenePirLigtSensor(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcScenePirLigtSensor");
	int statusRsp = CODE_OK;
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		uint16_t luxLow = 0;
		uint16_t luxHigh = 0;
		uint8_t pir = 0;
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["CMD"] = "SCENE_FOR_SENSOR_LIGHT_PIR";
		Json::Value data = reqValue["DATA"];
		if (data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString())
		{
			string deviceId = data["DEVICE_ID"].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (data.isMember("HANG_ON_TIME") && data["HANG_ON_TIME"].isInt())
				{
					if (bleProtocol)
					{
						if (bleProtocol->TimeActionPirLightSensor(device->GetAddr(), data["HANG_ON_TIME"].asInt()) != CODE_OK)
						{
							statusRsp = CODE_ERROR;
						}
					}
				}
				if (data.isMember("SCENE_ID") && data["SCENE_ID"].isString())
				{
					string sceneId = data["SCENE_ID"].asString();
					data["EVENT_TRIGGER_ID"] = sceneId;
					SceneBle *sceneBle = getSceneBleFromId(sceneId);
					if (sceneBle)
					{
						if (data.isMember("LUX") && data["LUX"].isArray() && data.isMember("PIR_VALUE") && data["PIR_VALUE"].isInt())
						{
							pir = data["PIR_VALUE"].asInt();
							if (data["LUX"].size() == 2 && data["LUX"][0].isInt() && data["LUX"][1].isInt())
							{
								luxLow = data["LUX"][0].asInt();
								luxHigh = data["LUX"][1].asInt();
								if (bleProtocol)
								{
									if (bleProtocol->SetScenePirLightSensor(device->GetAddr(), 2, pir, luxLow, luxHigh, sceneBle->GetAddr(), 0) != CODE_OK)
									{
										statusRsp = CODE_ERROR;
									}
								}
							}
						}
					}
					else
					{
						LOGW("Scene %s does not exsit", sceneId.c_str());
					}
				}
				if (data.isMember("AFTER_PIR_SCENE_ID") && data["AFTER_PIR_SCENE_ID"].isString())
				{
					string sceneAfterId = data["AFTER_PIR_SCENE_ID"].asString();
					SceneBle *sceneAfter = getSceneBleFromId(sceneAfterId);
					if (sceneAfter)
					{
						if (bleProtocol)
						{
							if (bleProtocol->SetScenePirLightSensor(device->GetAddr(), 2, 0, luxLow, luxHigh, sceneAfter->GetAddr(), 0) != CODE_OK)
							{
								statusRsp = CODE_ERROR;
							}
						}
					}
					else
					{
						LOGW("Scene %s does not exsit", sceneAfterId.c_str());
					}
				}
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
		dataJsonRsp["DATA"] = data;
	}
	return statusRsp;
}
int Gateway::OnRpcEditScenePirLightSensor(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcScenePirLigtSensor");
	int statusRsp = CODE_OK;
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		uint16_t luxLow = 0;
		uint16_t luxHigh = 0;
		uint8_t pir = 0;
		Json::Value dataJsonRsp = Json::objectValue;
		dataJsonRsp["CMD"] = "EDIT_SCENE_FOR_SENSOR_LIGHT_PIR";
		Json::Value data = reqValue["DATA"];
		if (data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString())
		{
			string deviceId = data["DEVICE_ID"].asString();
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				if (data.isMember("HANG_ON_TIME") && data["HANG_ON_TIME"].isInt())
				{
					if (bleProtocol)
					{
						if (bleProtocol->TimeActionPirLightSensor(device->GetAddr(), data["HANG_ON_TIME"].asInt()) != CODE_OK)
						{
							statusRsp = CODE_ERROR;
						}
					}
				}
				if (data.isMember("SCENE_ID") && data["SCENE_ID"].isString())
				{
					string sceneId = data["SCENE_ID"].asString();
					data["EVENT_TRIGGER_ID"] = sceneId;
					SceneBle *sceneBle = getSceneBleFromId(sceneId);
					if (sceneBle)
					{
						if (data.isMember("LUX") && data["LUX"].isArray() && data.isMember("PIR_VALUE") && data["PIR_VALUE"].isInt())
						{
							pir = data["PIR_VALUE"].asInt();
							if (data["LUX"].size() == 2 && data["LUX"][0].isInt() && data["LUX"][1].isInt())
							{
								luxLow = data["LUX"][0].asInt();
								luxHigh = data["LUX"][1].asInt();
								if (bleProtocol)
								{
									if (bleProtocol->SetScenePirLightSensor(device->GetAddr(), 2, pir, luxLow, luxHigh, sceneBle->GetAddr(), 0) != CODE_OK)
									{
										statusRsp = CODE_ERROR;
									}
								}
							}
						}
					}
					else
					{
						LOGW("Scene %s does not exsit", sceneId.c_str());
					}
				}
				if (data.isMember("AFTER_PIR_SCENE_ID") && data["AFTER_PIR_SCENE_ID"].isString())
				{
					string sceneAfterId = data["AFTER_PIR_SCENE_ID"].asString();
					SceneBle *sceneAfter = getSceneBleFromId(sceneAfterId);
					if (sceneAfter)
					{
						if (bleProtocol)
						{
							if (bleProtocol->SetScenePirLightSensor(device->GetAddr(), 2, 0, luxLow, luxHigh, sceneAfter->GetAddr(), 0) != CODE_OK)
							{
								statusRsp = CODE_ERROR;
							}
						}
					}
					else
					{
						LOGW("Scene %s does not exsit", sceneAfterId.c_str());
					}
				}
			}
			else
			{
				LOGW("Device %s does not exsit", deviceId.c_str());
			}
		}
		dataJsonRsp["DATA"] = data;
	}
	return statusRsp;
}
int Gateway::OnRpcRemoveScenePirLightSensor(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcRemoveScenePirLightSensor");
	int status = CODE_OK;
	if (reqValue.isMember("DATA") && reqValue["DATA"].isArray() && reqValue.isMember("DEVICE_ID") && reqValue["DEVICE_ID"].isString())
	{
		respValue["CMD"] = "REMOVE_SCENE_FOR_SENSOR_LIGHT_PIR";
		Json::Value data = reqValue["DATA"];
		string deviceId = reqValue["DEVICE_ID"].asString();
		// int pirValue = 0;
		string sceneId = "";
		SceneBle *scene = NULL;

		Device *device = getDeviceFromId(deviceId);
		if (device)
		{
			for (Json::ArrayIndex i = 0; i < data.size(); i++)
			{
				Json::Value dataValue = data[i];
				// if (dataValue.isMember("PIR_VALUE") && dataValue["PIR_VALUE"].isInt())
				// {
				// 	pirValue = dataValue["PIR_VALUE"].asInt();
				// }
				if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
				{
					sceneId = dataValue["EVENT_TRIGGER_ID"].asString();
					scene = getSceneBleFromId(sceneId);
					if (scene)
					{
						if (bleProtocol)
						{
							if (bleProtocol->DelScenePirLightSensor(device->GetAddr(), scene->GetAddr()) != CODE_OK)
								status = CODE_ERROR;
						}
					}
				}
			}
		}
		respValue["DATA"] = data;
	}
	else
	{
		status = CODE_ERROR;
		LOGW("OnRpcRemoveScenePirLightSensor error: %s", reqValue.toString().c_str());
	}
	return status;
}

int Gateway::OnRpcSceneScreen(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "SCENE_FOR_SCREEN";
		Json::Value data = reqValue["DATA"];
		Json::Value dataJson = Json::objectValue;
		if (data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString())
		{
			string deviceId = data["DEVICE_ID"].asString();
			dataJson["DEVICE_ID"] = deviceId;
			Device *device = getDeviceFromId(deviceId);
			string status = "FAILED";
			if (device && device->GetType() == BLE_AC_SCENE_SCREEN_TOUCH)
			{
				if (data.isMember("SCENES") && data["SCENES"].isArray())
				{
					Json::Value scenes = data["SCENES"];
					for (Json::ArrayIndex i = 0; i < scenes.size(); i++)
					{
						Json::Value scene = scenes[i];
						if (scene.isMember("SCENE_ID") && scene["SCENE_ID"].isString() && scene.isMember("SCENE_NAME") && scene["SCENE_NAME"].isString() && scene.isMember("SCENE_ICON") && scene["SCENE_ICON"].isInt())
						{
							string sceneId = scene["SCENE_ID"].asString();
							string sceneName = scene["SCENE_NAME"].asString();
							int sceneIcon = scene["SCENE_ICON"].asInt();
							SceneBle *scene = getSceneBleFromId(sceneId);
							if (scene)
							{
								if (bleProtocol)
								{
									if (bleProtocol->SceneForScreenTouch(device->GetAddr(), scene->GetAddr(), sceneIcon, 1) == CODE_OK)
									{
										status = "SUCCESS";
									}
								}
								else
									LOGW("BleProtocol null");
							}
							else
							{
								LOGW("Scene %s not found", sceneId.c_str());
							}
						}
					}
				}

				if (data.isMember("DEL_SCENES") && data["DEL_SCENES"].isArray())
				{
					Json::Value delScene = data["DEL_SCENES"];
					for (Json::ArrayIndex j = 0; j < delScene.size(); j++)
					{
						if (delScene[j].isString())
						{
							string sceneId = delScene[j].asString();
							SceneBle *sceneDel = getSceneBleFromId(sceneId);
							if (sceneDel)
							{
								if (bleProtocol)
								{
									if (bleProtocol->DelSceneScreenTouch(device->GetAddr(), sceneDel->GetAddr()) == CODE_OK)
									{
										status = "SUCCESS";
									}
								}
								else
									LOGW("BleProtocol null");
							}
							else
							{
								LOGW("Scene del %s not found", sceneId.c_str());
							}
						}
					}
				}
				dataJson["STATUS"] = status;
			}
			else
			{
				LOGW("Device %s is not supported", deviceId.c_str());
			}
		}
		respValue["DATA"] = dataJson;
		return CODE_OK;
	}
	LOGW("OnRpcRemoveScenePirLightSensor error: %s", reqValue.toString().c_str());
	return CODE_ERROR;
}

int Gateway::OnRpcStairsSwitch(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "STAIRS_SWITCH";
		Json::Value dataJsonRsp = Json::objectValue;
		string statusRsp = "SUCCESS";

		Json::Value data = reqValue["DATA"];
		if (data.isMember("ID") && data["ID"].isString() && data.isMember("LIST_BUTTON_LINK") && data["LIST_BUTTON_LINK"].isArray())
		{
			string groupId = data["ID"].asString();
			dataJsonRsp["ID"] = groupId;
			Group *group = getGroupFromId(groupId);
			if (group == NULL)
			{
				int groupAddr = 1;
				groupListMtx.lock();
				for (auto id = groupList.begin(); id != groupList.end(); ++id)
				{
					if (id->second->GetAddr() > groupAddr)
						groupAddr = id->second->GetAddr() + 1;
				}
				groupListMtx.unlock();
				group = new Group(groupId, groupAddr, groupId);
				group = AddNewGroup(group, true, true);
			}
			if (group)
			{
				for (Json::ArrayIndex i = 0; i < data["LIST_BUTTON_LINK"].size(); i++)
				{
					Json::Value deviceLink = data["LIST_BUTTON_LINK"][i];
					if (deviceLink.isMember("DEVICE_ID") && deviceLink["DEVICE_ID"].isString() && deviceLink.isMember("BUTTON_ID") && deviceLink["BUTTON_ID"].isInt())
					{
						string deviceLinkId = deviceLink["DEVICE_ID"].asString();
						int buttonId = deviceLink["BUTTON_ID"].asInt();
						Device *deviceParent = getDeviceFromId(deviceLinkId);
						if (deviceParent)
						{
							uint32_t addrParent = deviceParent->GetAddr();
							Device *deviceChild = getDeviceBleFromAddr(addrParent + (buttonId - 11));
							if (deviceChild)
							{
								uint32_t addrChild = deviceChild->GetAddr();
								if (group->AddDevice(deviceParent, addrChild, true) != CODE_OK)
								{
									statusRsp = "FAILED";
								}
								database->DeviceInGroupAdd(group, deviceParent, addrChild);
								if (bleProtocol)
								{
									if (bleProtocol->SetIdCombine(addrChild, group->GetAddr() + 49152) != CODE_OK)
										statusRsp = "FAILED";
								}
								else
								{
									statusRsp = "FAILED";
									LOGW("BLEProtocol null");
								}
							}
							else
								LOGW("Device %s not found", deviceChild->GetId().c_str());
						}
						else
							LOGW("Device %s not found", deviceLinkId.c_str());
					}
				}
			}
			else
			{
				LOGW("Group %s not found", groupId.c_str());
				statusRsp = "FAILED";
			}
		}
		dataJsonRsp["STATUS"] = statusRsp;
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcEditStairsSwitch(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "EDIT_STAIRS_SWITCH";
		Json::Value dataJsonRsp = Json::objectValue;
		string statusRsp = "SUCESS";

		Json::Value &data = reqValue["DATA"];
		if (data.isMember("ID") && data["ID"].isString())
		{
			string groupId = data["ID"].asString();
			dataJsonRsp["ID"] = groupId;
			Group *group = getGroupFromId(groupId);
			if (group)
			{
				if (data.isMember("ADD_BUTTON") && data["ADD_BUTTON"].isArray())
				{
					for (Json::ArrayIndex i = 0; i < data["ADD_BUTTON"].size(); i++)
					{
						Json::Value deviceAdd = data["ADD_BUTTON"][i];
						if (deviceAdd.isMember("DEVICE_ID") && deviceAdd["DEVICE_ID"].isString() && deviceAdd.isMember("BUTTON_ID") && deviceAdd["BUTTON_ID"].isInt())
						{
							string deviceAddId = deviceAdd["DEVICE_ID"].asString();
							int buttonId = deviceAdd["BUTTON_ID"].asInt();
							Device *deviceAddParent = getDeviceFromId(deviceAddId);
							if (deviceAddParent)
							{
								uint32_t addrParent = deviceAddParent->GetAddr();
								Device *deviceAddChild = getDeviceBleFromAddr(addrParent + (buttonId - 11));
								if (deviceAddChild)
								{
									uint32_t addrChild = deviceAddChild->GetAddr();
									if (group->AddDevice(deviceAddParent, addrChild, true) != CODE_OK)
									{
										statusRsp = "FAILED";
									}
									database->DeviceInGroupAdd(group, deviceAddParent, addrChild);

									if (bleProtocol)
									{
										if (bleProtocol->SetIdCombine(addrChild, group->GetAddr() + 49152) != CODE_OK)
											statusRsp = "FAILED";
									}
									else
									{
										statusRsp = "FAILED";
										LOGW("BLEProtocol null");
									}
								}
								else
									LOGW("Device %s not found", deviceAddChild->GetId().c_str());
							}
							else
								LOGW("Device %s not found", deviceAddId.c_str());
						}
					}
				}

				if (data.isMember("REMOVE_BUTTON") && data["REMOVE_BUTTON"].isArray())
				{
					for (Json::ArrayIndex i = 0; i < data["REMOVE_BUTTON"].size(); i++)
					{
						Json::Value deviceRemove = data["REMOVE_BUTTON"][i];
						if (deviceRemove.isMember("DEVICE_ID") && deviceRemove["DEVICE_ID"].isString() && deviceRemove.isMember("BUTTON_ID") && deviceRemove["BUTTON_ID"].isInt())
						{
							string deviceRemoveId = deviceRemove["DEVICE_ID"].asString();
							int buttonId = deviceRemove["BUTTON_ID"].asInt();
							Device *deviceRemoveParent = getDeviceFromId(deviceRemoveId);
							if (deviceRemoveParent)
							{
								uint32_t addrParent = deviceRemoveParent->GetAddr();
								Device *deviceRemoveChild = getDeviceBleFromAddr(addrParent + (buttonId - 11));
								if (deviceRemoveChild)
								{
									uint32_t addrChild = deviceRemoveChild->GetAddr();
									if (group->DelDevice(deviceRemoveParent, addrChild) != CODE_OK)
									{
										statusRsp = "FAILED";
									}
									database->DeviceInGroupDel(group, deviceRemoveParent, addrChild);

									if (bleProtocol)
									{
										if (bleProtocol->SetIdCombine(addrChild, 0) != CODE_OK)
											statusRsp = "FAILED";
									}
									else
									{
										statusRsp = "FAILED";
										LOGW("BLEProtocol null");
									}
								}
								else
									LOGW("Device %s not found", deviceRemoveChild->GetId().c_str());
							}
							else
								LOGW("Device %s not found", deviceRemoveId.c_str());
						}
					}
				}
			}
			else
			{
				LOGW("Group %s not found", groupId.c_str());
				statusRsp = "FAILED";
			}
		}
		dataJsonRsp["STATUS"] = statusRsp;
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcDelStairsSwitch(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value &data = reqValue["DATA"];
		respValue["CMD"] = "DELETE_STAIRS_SWITCH";
		Json::Value dataJsonRsp = Json::objectValue;
		string statusRsp = "SUCCESS";

		if (data.isMember("GROUP_ID") && data["GROUP_ID"].isString())
		{
			string groupId = data["GROUP_ID"].asString();
			dataJsonRsp["ID"] = groupId;
			Group *group = getGroupFromId(groupId);
			if (group)
			{
				int numDevices = group->deviceList.size();
				vector<DeviceInGroup *> list = group->deviceList;
				// for (int j = 0; j < numDevices; j++)
				// {
				// 	LOGE("Device %s, epid: %d", list[j]->device->GetId(), list[j]->epId);
				// }
				for (int i = 0; i < numDevices; i++)
				{
					if (group->DelDevice(list[i]->device, list[i]->epId) != CODE_OK)
					{
						statusRsp = "FAILED";
					}
					database->DeviceInGroupDel(group, list[i]->device, list[i]->epId);

					if (bleProtocol)
					{
						if (bleProtocol->SetIdCombine(list[i]->device->GetAddr(), 0) != CODE_OK)
							statusRsp = "FAILED";
					}
					else
					{
						statusRsp = "FAILED";
						LOGW("BLEProtocol null");
					}
				}
				delGroup(group);
			}
			else
			{
				LOGW("Group %s not found", groupId.c_str());
				statusRsp = "FAILED";
			}
		}
		dataJsonRsp["STATUS"] = statusRsp;
		respValue["DATA"] = dataJsonRsp;
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcPowerSwitchTimeout(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcPowerSwitchTimeout");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "POWER_SWITCH_TIMEOUT";
		Json::Value data = reqValue["DATA"];
		respValue["DATA"] = data;

		if (data.isMember("EVENT_TRIGGER_ID") && data["EVENT_TRIGGER_ID"].isString() &&
			data.isMember("DEVICE_ID") && data["DEVICE_ID"].isString() &&
			data.isMember("BUTTON") && data["BUTTON"].isString() &&
			data.isMember("CHANGE_AT") && data["CHANGE_AT"].isString() &&
			data.isMember("CHANGE_TO") && data["CHANGE_TO"].isInt())
		{
			string id = data["EVENT_TRIGGER_ID"].asString();
			string devId = data["DEVICE_ID"].asString();
			string btnId = data["BUTTON"].asString();
			string time = data["CHANGE_AT"].asString();
			int value = data["CHANGE_TO"].asInt();

			Json::Value properties;
			properties["ID"] = 0;
			properties["VALUE"] = value;

			uint8_t buttonId = 11;
			if (btnId == "BUTTON_1")
				buttonId = 11;
			else if (btnId == "BUTTON_2")
				buttonId = 12;
			else if (btnId == "BUTTON_3")
				buttonId = 13;
			else if (btnId == "BUTTON_4")
				buttonId = 14;
			else if (btnId == "BUTTON_5")
				buttonId = 15;
			else if (btnId == "BUTTON_6")
				buttonId = 16;

			Device *deviceParent = getDeviceFromId(devId);
			Device *deviceChild = NULL;
			if (deviceParent)
			{
				deviceChild = getDeviceBleFromAddr(deviceParent->GetAddr() + (buttonId - 11));
			}
			else
				LOGW("Device not found");

			Rule *rule = getRuleFromId(id);
			if (rule)
			{
				rule->DelAllRuleInput();
				rule->DelAllRuleOutput();
			}
			if (deviceChild)
			{
				int day = Util::GetDaysCurrent();
				int mon = 0, tue = 0, wed = 0, thu = 0, fri = 0, sat = 0, sun = 0;
				switch (day)
				{
				case 2:
					mon = 1;
					break;
				case 3:
					tue = 1;
					break;
				case 4:
					wed = 1;
					break;
				case 5:
					thu = 1;
					break;
				case 6:
					fri = 1;
					break;
				case 7:
					sat = 1;
					break;
				case 8:
					sun = 1;
					break;
				}
				int repeat = Util::ConvertRepeatDayToInt(mon, tue, wed, thu, fri, sat, sun);
				rule = new Rule(id, "and", repeat, "", 0, Util::ConvertStrTimeToInt(time), Util::ConvertStrTimeToInt(""), reqValue);
				RuleOutputDevice *ruleOutputDevice = new RuleOutputDevice(deviceChild, properties, 0);
				rule->AddRuleOutput(ruleOutputDevice);
				ruleListMtx.lock();
				ruleList[id] = rule;
				ruleListMtx.unlock();
			}
			else
				LOGW("Device not found");
			return CODE_OK;
		}
		else
			LOGW("Power switch timeout failed");
	}
	return CODE_ERROR;
}
int Gateway::OnRpcRemovePowerSwitchTimeout(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "REMOVE_POWER_SWITCH_TIMEOUT";
		Json::Value dataValue = reqValue["DATA"];
		respValue["DATA"] = dataValue;
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string ruleId = dataValue["EVENT_TRIGGER_ID"].asString();
			ruleListMtx.lock();
			ruleList.erase(ruleList.find(ruleId));
			ruleListMtx.unlock();
			return CODE_OK;
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}
/**
 * @brief
 *
 * @param reqValue
 * @param respValue
 * @return int
 */
int Gateway::OnRpcAddDevice(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("id") && dataValue["id"].isString() &&
			dataValue.isMember("name") && dataValue["name"].isString() &&
			dataValue.isMember("mac") && dataValue["mac"].isString() &&
			dataValue.isMember("addr") && dataValue["addr"].isInt() &&
			dataValue.isMember("type") && dataValue["type"].isInt() &&
			dataValue.isMember("devicekey") && dataValue["devicekey"].isString() &&
			dataValue.isMember("version") && dataValue["version"].isInt())
		{
			string deviceId = dataValue["id"].asString();
			string name = dataValue["name"].asString();
			string mac = dataValue["mac"].asString();
			uint32_t addr = dataValue["addr"].asInt();
			uint32_t type = dataValue["type"].asInt();
			string devicekey = dataValue["devicekey"].asString();
			uint16_t version = dataValue["version"].asInt();
			Device *device = AddNewDevice(deviceId, name, mac, devicekey, addr, type, version, true, true);
			respValue["code"] = 0;
			return CODE_OK;
		}
	}
	respValue["code"] = -1;
	return CODE_ERROR;
}

int Gateway::OnRpcAddTuyaDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcAddTuyaDevice");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() &&
			dataValue.isMember("PROPERTIES") && dataValue["PROPERTIES"].isArray())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			Json::Value properties = dataValue["PROPERTIES"];
			LOGI("New Tuya Device: %s", deviceId.c_str());
			// Device *device = getDeviceFromId(deviceId);
			// if (device)
			// {
			// 	for (Json::ArrayIndex i = 0; i < properties.size(); i++)
			// 	{
			// 		Json::Value property = properties[i];
			// 		if (property.isObject() &&
			// 				property.isMember("ID") && property["ID"].isInt() &&
			// 				property.isMember("CMD") && property["CMD"].isString())
			// 		{
			// 			int id = property["ID"].asInt();
			// 			string cmd = property["CMD"].asString();
			// 			device->DoJsonArrayDo(id, value);
			// 		}
			// 	}
			// }
			// else
			// {
			// 	LOGW("Device not found");
			// }
		}
		else
		{
			LOGW("Format error");
		}
	}
	else
	{
		LOGW("Format error");
	}
	respValue = reqValue;
	return CODE_OK;
}

//
int Gateway::OnRpcDelAllDevice(Json::Value &reqValue, Json::Value &respValue)
{
	database->DeviceDelAll();
	deviceList.clear();
	if (bleProtocol)
	{
		bleProtocol->ResetFactory();
	}
	else
		LOGW("BleProtocol null");
	respValue["code"] = 0;
	return CODE_OK;
}

int Gateway::OnRpcControlDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcControlDevice");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() &&
			dataValue.isMember("PROPERTIES") && dataValue["PROPERTIES"].isArray())
		{
			string deviceId = dataValue["DEVICE_ID"].asString();
			Json::Value properties = dataValue["PROPERTIES"];
			Device *device = getDeviceFromId(deviceId);
			if (device)
			{
				device->DoJsonArray(properties);
				// respValue = reqValue;
				// return CODE_OK;
			}
			else
			{
				LOGW("Device not found");
			}
		}
		else
		{
			LOGW("Format error");
		}
	}
	else
	{
		LOGW("Format error");
	}
	return CODE_NOT_RESPONSE;
}

int Gateway::OnRpcControlGroup(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("GROUP_ID") && dataValue["GROUP_ID"].isString() &&
			dataValue.isMember("PROPERTIES") && dataValue["PROPERTIES"].isArray())
		{
			string groupId = dataValue["GROUP_ID"].asString();
			Json::Value properties = dataValue["PROPERTIES"];
			Group *group = getGroupFromId(groupId);
			if (group)
			{
				if (properties.isArray())
				{
					group->Do(properties);
				}
			}
			else
			{
				LOGW("Group not found");
			}
		}
		else
		{
			LOGW("Format error");
		}
	}
	else
	{
		LOGW("Format error");
	}
	return CODE_NOT_RESPONSE;
}

int Gateway::OnRpcUpdateAllTelemetry(Json::Value &reqValue, Json::Value &respValue)
{
	Json::Value dataValue;
	Json::Value deviceValue;
	Json::Value propertiesValue;
	for (const auto &[id, device] : deviceList)
	{
		propertiesValue = Json::Value::null;
		deviceValue = Json::Value::null;
		device->BuildTelemetryValue(propertiesValue);
		deviceValue["DEVICE_ID"] = device->GetId();
		deviceValue["PROPERTIES"] = propertiesValue;
		dataValue.append(deviceValue);
	}
	respValue["CMD"] = "DEVICE_UPDATE";
	respValue["DATA"] = dataValue;
	return CODE_OK;
}

int Gateway::OnRpcControlSceneBle(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString())
		{
			string sceneId = dataValue["SCENE_ID"].asString();
			SceneBle *scene = getSceneBleFromId(sceneId);
			if (scene)
			{
				scene->Do();
				return CODE_NOT_RESPONSE;
			}

			SceneDelay *sceneDelay = getSceneDelayFromId(sceneId);
			if (sceneDelay)
			{
				sceneDelay->RunOutput();
				return CODE_NOT_RESPONSE;
			}

			LOGW("Scene %s dose not exsit", sceneId.c_str());
		}
	}
	return CODE_NOT_RESPONSE;
}

int Gateway::OnRpcSetPwMqttOnline(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "SET_PASSWD_MQTT_ONLINE";
		Json::Value dataJsonRsp = Json::objectValue;
		int status = 0;
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("PASSWD") && dataValue["PASSWD"].isString())
		{
			string password = dataValue["PASSWD"].asString();
			string user = "";

#ifdef ESP_PLATFORM
			user = "minihub-" + mac;
#else
			user = "hc-" + mac;
#endif

			if (config->SetClientId(user))
			{
				if (config->SetUsername(user))
				{
					if (config->SetPort(1884))
					{
						if (config->SetPassword(password))
						{
							status = 1;
						}
						else
						{
							status = 0;
						}
					}
					else
					{
						status = 0;
					}
				}
				else
				{
					status = 0;
				}
			}
			else
			{
				status = 0;
			}
			dataJsonRsp["STATUS"] = status;
			respValue["DATA"] = dataJsonRsp;
			return CODE_EXIT;
		}
	}
	return CODE_ERROR;
}

int Gateway::OnRpcSSHRemote(Json::Value &reqValue, Json::Value &respValue)
{
	int err = 0;
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value dataValue = reqValue["params"];
		if (dataValue.isMember("type") && dataValue["type"].isString() &&
			dataValue.isMember("key") && dataValue["key"].isString() &&
			dataValue.isMember("user") && dataValue["user"].isString() &&
			dataValue.isMember("host") && dataValue["host"].isString() &&
			dataValue.isMember("serverPort") && dataValue["serverPort"].isInt() &&
			dataValue.isMember("forwardPort") && dataValue["forwardPort"].isInt())
		{
			string key = "";
			string type = dataValue["type"].asString();
			string user = dataValue["user"].asString();
			string host = dataValue["host"].asString();
			uint32_t serverPort = dataValue["serverPort"].asInt();
			uint32_t forwardPort = dataValue["forwardPort"].asInt();
			uint32_t localPort = 22;
			if (dataValue.isMember("localPort") && dataValue["localPort"].isInt())
			{
				localPort = dataValue["localPort"].asInt();
			}
			if (type == "base64")
			{
				string keyBase64 = dataValue["key"].asString();
				string decode = macaron::Base64::Decode(keyBase64, key);
				if (decode != "")
				{
					err = 1;
					LOGW("Base64 decode err: %s", decode.c_str());
				}
			}
			else
			{
				key = dataValue["key"].asString();
			}

			if (err == 0)
			{
				// save key file
				system("rm /key.txt");
				system("rm /output.txt");
				ofstream keyFile("/key.txt");
				keyFile << key;
				keyFile.close();

				system("chmod 600 /key.txt");
				system("killall ssh");
				string cmd = "ssh -i /key.txt -o StrictHostKeyChecking=no -f -N -T -R" + to_string(forwardPort) + ":localhost:" + to_string(localPort) + " " + user + "@" + host + " -p " + to_string(serverPort);
				cmd += " >> /output.txt 2>&1";
				LOGI("cmd: %s", cmd.c_str());
				system(cmd.c_str());
				sleep(2);
				bool err = false;
				FILE *fp = fopen("/output.txt", "r");
				char path[512] = {0};
				if (fp)
				{
					while (fgets(path, sizeof(path), fp) != NULL)
					{
						if (strlen(path) > 1)
						{
							LOGW("SSH err: %s", path);
							err = true;
							break;
						}
					}
					fclose(fp);
				}
				if (err)
				{
					respValue["msg"] = string(path);
					respValue["code"] = 1;
				}
				else
				{
					respValue["code"] = 0;
				}
				return CODE_OK;
			}
		}
	}
	respValue["code"] = err;
	return CODE_OK;
}

int Gateway::OnRpcAddDeviceSmartHomeToRoom(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value dataValue = reqValue["DATA"];
		if (dataValue.isMember("DEVICE_ID") && dataValue["DEVICE_ID"].isString() &&
			dataValue.isMember("ROOM_ID") && dataValue["ROOM_ID"].isString())
		{
			int deviceType;
			string deviceId = dataValue["DEVICE_ID"].asString();
			string roomId = dataValue["ROOM_ID"].asString();
			DeviceBle *device = (DeviceBle *)gateway->getDeviceFromId(deviceId);
			Group *group = getGroupFromId(roomId);
			if (group)
			{
				Room *room = getRoomFromId(roomId);
				if (room)
				{
					if (device)
					{
						deviceType = device->GetType();
						if (deviceType == BLE_REMOTE_M3_V2 || deviceType == BLE_REMOTE_M4 || deviceType == BLE_AC_SCENE_SCREEN_TOUCH)
						{
							if (room->AddDevice(device, true) == CODE_OK)
							{
								database->DeviceInRoomAdd(room, device);
							}
						}
						else
						{
							if (room->AddDevice(device, false) == CODE_OK)
							{
								database->DeviceInRoomAdd(room, device);
							}
						}
					}
					else
					{
						LOGW("Device %s is does not exist", deviceId.c_str());
					}
				}
				else
				{
					room = new Room(roomId, group->GetAddr(), "");
					if (room)
					{
						Room *roomAddGw = AddNewRoom(room, true, true);
						if (roomAddGw)
						{
							if (device)
							{
								deviceType = device->GetType();
								if (deviceType == BLE_REMOTE_M3_V2 || deviceType == BLE_REMOTE_M4 || deviceType == BLE_AC_SCENE_SCREEN_TOUCH)
								{
									if (roomAddGw->AddDevice(device, true) == CODE_OK)
									{
										database->DeviceInRoomAdd(roomAddGw, device);
									}
								}
								else
								{
									LOGW("Device type does not support add room");
								}
							}
							else
							{
								LOGW("Device %s is does not exist", deviceId.c_str());
							}
						}
					}
				}
			}
			else
			{
				LOGW("room %s does not exist", roomId.c_str());
			}
		}
	}
	return CODE_OK;
}

int Gateway::OnRpcCreateCountDown(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "COUNTDOWN";
		Json::Value dataValue = reqValue["DATA"];
		respValue["DATA"] = dataValue;
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString() && dataValue.isMember("START_AT") && dataValue["START_AT"].isString() && dataValue.isMember("SCENE_ID") && dataValue["SCENE_ID"].isString())
		{
			string name;
			uint32_t addr = 0;
			string eventTriggerId = dataValue["EVENT_TRIGGER_ID"].asString();
			string startAt = dataValue["START_AT"].asString();
			string sceneId = dataValue["SCENE_ID"].asString();
			SceneBle *scene = getSceneBleFromId(sceneId);
			Rule *rule = getRuleFromId(eventTriggerId);
			if (rule)
			{
				rule->DelAllRuleInput();
				rule->DelAllRuleOutput();
			}
			if (scene)
			{
				int day = Util::GetDaysCurrent();
				int mon = 0, tue = 0, wed = 0, thu = 0, fri = 0, sat = 0, sun = 0;
				switch (day)
				{
				case 2:
					mon = 1;
					break;
				case 3:
					tue = 1;
					break;
				case 4:
					wed = 1;
					break;
				case 5:
					thu = 1;
					break;
				case 6:
					fri = 1;
					break;
				case 7:
					sat = 1;
					break;
				case 8:
					sun = 1;
					break;
				}
				int repeat = Util::ConvertRepeatDayToInt(mon, tue, wed, thu, fri, sat, sun);
				rule = new Rule(eventTriggerId, "and", repeat, "", 0, Util::ConvertStrTimeToInt(startAt), Util::ConvertStrTimeToInt(""), reqValue);
				RuleOutputSceneBle *ruleOutputSceneBle = new RuleOutputSceneBle(scene, 0);
				rule->AddRuleOutput(ruleOutputSceneBle);
				ruleListMtx.lock();
				ruleList[eventTriggerId] = rule;
				ruleListMtx.unlock();
			}
			else
			{
				LOGW("Scene %s not found", sceneId.c_str());
			}
			return CODE_OK;
		}
		else
		{
			LOGW("CountDown failed");
		}
	}
	return CODE_ERROR;
}

int Gateway::OnRpcDelCountDown(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		respValue["CMD"] = "DELETE_COUNTDOWN";
		Json::Value dataValue = reqValue["DATA"];
		respValue["DATA"] = dataValue;
		if (dataValue.isMember("EVENT_TRIGGER_ID") && dataValue["EVENT_TRIGGER_ID"].isString())
		{
			string ruleId = dataValue["EVENT_TRIGGER_ID"].asString();
			ruleListMtx.lock();
			ruleList.erase(ruleList.find(ruleId));
			ruleListMtx.unlock();
			return CODE_OK;
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Gateway::OnRpcUpdateFirmware(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCUpdateFirmware");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isArray())
	{
		Json::Value datasValue = reqValue["DATA"];
		string nameOld = "";
		string name;
		string sum;
		string url;
		int numFirm = datasValue.size();
		if (numFirm > 0)
		{
			Json::Value dataValue = datasValue[numFirm - 1];
			if (dataValue.isMember("NAME") && dataValue["NAME"].isString() &&
				dataValue.isMember("CHECK_SUM") && dataValue["CHECK_SUM"].isString() &&
				dataValue.isMember("URL") && dataValue["URL"].isString())
			{
				name = dataValue["NAME"].asString();
				sum = dataValue["CHECK_SUM"].asString();
				url = dataValue["URL"].asString();
				string check = name + ".tar.xz";

#ifdef ESP_PLATFORM

				if (url.find("smh_gw.bin") != std::string::npos)
				{
					nameOld = name;
				}
#else
				if (url.find(check) != std::string::npos)
				{
					if (name > nameOld)
					{
						nameOld = name;
					}
				}
#endif
			}
		}
		if (nameOld != "")
		{
			LOGD("name: %s, url: %s, sum: %s", name.c_str(), url.c_str(), sum.c_str());
			string domain = string(BASE_URL_DEV) + url;

			DelAllDevice();
			DelAllGroup();
			DelAllRoom();
			DelAllRule();
			DelAllSceneBle();
			DelAllSceneDelay();
#ifdef ESP_PLATFORM
			config->SetUrlOta(domain);
			config->SetCheckSumOta(sum);
			esp_restart();
#endif
			Ota::startOta(name, domain, sum);
			return CODE_OK;
		}
	}
	else
	{
		LOGW("Format error");
	}
	return CODE_ERROR;
}
