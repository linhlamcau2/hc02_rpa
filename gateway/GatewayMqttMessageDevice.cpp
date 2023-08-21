#include "Gateway.h"
#include "Log.h"
#include "Db.h"

void Gateway::InitMqttMessageDevice()
{
	OnDeviceRpcCallbackRegister("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getDevStt", bind(&Gateway::OnGetDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getAllDevStt", bind(&Gateway::OnGetAllDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getDevList", bind(&Gateway::OnGetDeviceList, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getDevStt", bind(&Gateway::OnGetDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getAllDevStt", bind(&Gateway::OnGetAllDeviceStatus, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getDevList", bind(&Gateway::OnGetDeviceList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getCamList", bind(&Gateway::OnGetCamList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("newDev", bind(&Gateway::OnNewDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delDev", bind(&Gateway::OnDeleteDevice, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("addFavoriteDev", bind(&Gateway::OnAddFavoriteDev, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delFavoriteDev", bind(&Gateway::OnDelFavoriteDev, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getFavoriteDev", bind(&Gateway::OnGetFavoriteDev, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("updateDeviceName", bind(&Gateway::OnUpdateDeviceName, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnControlDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlDevice");
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("data") && reqValue["data"].isObject())
	{
		string deviceId = reqValue["id"].asString();
		Json::Value devData = reqValue["data"];
		Device *device = getDeviceFromId(deviceId);
		if (device)
		{
			int rs = device->Do(devData);
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
	respValue["cmd"] = "controlDevRsp";
	return CODE_OK;
}

int Gateway::OnControlAllDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlAllDevice");
	if (bleProtocol)
	{
		if (reqValue.isMember(KEY_ATTRIBUTE_ONOFF) && reqValue[KEY_ATTRIBUTE_ONOFF].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_ONOFF].asInt();
			bleProtocol->SetOnOffLight(0xFFFF, value, 0, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_DIM) && reqValue[KEY_ATTRIBUTE_DIM].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_DIM].asInt();
			uint16_t dim = (value * 65535) / 100;
			bleProtocol->SetDimmingLight(0xFFFF, dim, 0, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_CCT) && reqValue[KEY_ATTRIBUTE_CCT].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_CCT].asInt();
			uint16_t cct = (value * 192) + 800;
			bleProtocol->SetCctLight(0xFFFF, cct, 0, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_HUE) && reqValue[KEY_ATTRIBUTE_HUE].isInt() &&
			reqValue.isMember(KEY_ATTRIBUTE_SATURATION) && reqValue[KEY_ATTRIBUTE_SATURATION].isInt() &&
			reqValue.isMember(KEY_ATTRIBUTE_LUMINANCE) && reqValue[KEY_ATTRIBUTE_LUMINANCE].isInt())
		{
			int h = reqValue[KEY_ATTRIBUTE_HUE].asInt();
			int s = reqValue[KEY_ATTRIBUTE_SATURATION].asInt();
			int l = reqValue[KEY_ATTRIBUTE_LUMINANCE].asInt();
			bleProtocol->SetHSLLight(0xFFFF, h, s, l, 0, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && reqValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
			bleProtocol->CallModeRgb(0xFFFF, value);
		}
		respValue["data"]["code"] = CODE_OK;
	}
	respValue["cmd"] = "controlAllDevRsp";
	return CODE_OK;
}

int Gateway::OnGetDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetDeviceStatus");
	if (reqValue.isMember("devices") && reqValue["devices"].isArray())
	{
		Json::Value devicesValueRsp;
		Json::Value devicesValue = reqValue["devices"];
		for (auto &deviceValue : devicesValue)
		{
			if (deviceValue.isString())
			{
				string deviceId = deviceValue.asString();
				Device *device = getDeviceFromId(deviceId);
				if (device)
				{
					Json::Value deviceValue;
					deviceValue["id"] = device->GetId();
					Json::Value deviceAttbute;
					device->BuildTelemetryValue(deviceAttbute);
					deviceValue["data"] = deviceAttbute;
					devicesValueRsp.append(deviceValue);
				}
			}
		}
		respValue["data"]["code"] = CODE_OK;
		respValue["data"]["device"] = devicesValueRsp;
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "deviceUpdate";
	return CODE_OK;
}

int Gateway::OnGetAllDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetAllDeviceStatus");
	Json::Value devicesValueRsp;
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		Json::Value deviceValue;
		deviceValue["id"] = device->GetId();
		Json::Value deviceAttbute;
		device->BuildTelemetryValue(deviceAttbute);
		deviceValue["data"] = deviceAttbute;
		devicesValueRsp.append(deviceValue);
	}
	deviceListMtx.unlock();
	respValue["data"]["code"] = CODE_OK;
	respValue["data"]["device"] = devicesValueRsp;
	respValue["cmd"] = "deviceUpdate";
	return CODE_OK;
}

int Gateway::OnGetDeviceList(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetDeviceList");
	Json::Value devicesValueRsp;
	for (const auto &[id, device] : deviceList)
	{
		Json::Value deviceValue;
		deviceValue["id"] = device->GetId();
		deviceValue["addr"] = (Json::UInt)device->GetAddr();
		deviceValue["type"] = (Json::UInt)device->GetType();
		deviceValue["mac"] = device->GetMac();
		deviceValue["ver"] = device->GetVersionStr();
		devicesValueRsp.append(deviceValue);
	}
	respValue["data"]["devices"] = devicesValueRsp;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getDevListRsp";
	return CODE_OK;
}

int Gateway::OnGetCamList(Json::Value &reqValue, Json::Value &respValue)
{
	LOGE("OnGetCamList");
	Json::Value roomData;
	for (const auto &[id, room] : roomList)
	{
		Json::Value temp_devicesList;
		for (unsigned int i = 0; i < room->deviceList.size(); i++)
		{
			Json::Value dvList;
			DeviceInGroup *deviceInRoom = room->deviceList[i];
			int type = deviceInRoom->device->GetType();
			if (type / 10000 == 6)
			{
				Json::Value deviceValue;
				deviceValue["id"] = deviceInRoom->device->GetId();
				deviceValue["mac"] = deviceInRoom->device->GetMac();
				deviceValue["data"] = deviceInRoom->device->GetData();
				temp_devicesList["camList"].append(deviceValue);
			}
		}
		temp_devicesList["id"] = id;
		roomData.append(temp_devicesList);
	}
	respValue["data"]["room"] = roomData;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getCamListRsp";
	return CODE_OK;
}

int Gateway::OnNewDevice(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("device") && reqValue["device"].isArray())
	{
		Json::Value device = reqValue["device"];
		for (auto &temp : device)
		{
			if (temp.isMember("id") && temp["id"].isString() &&
				temp.isMember("name") && temp["name"].isString() &&
				temp.isMember("mac") && temp["mac"].isString() &&
				temp.isMember("type") && temp["type"].isInt() &&
				temp.isMember("data") && temp["data"].isObject())
			{
				string id = temp["id"].asString();
				string name = temp["name"].asString();
				string mac = temp["mac"].asString();
				int type = temp["type"].asInt();
				AddNewDevice(id, name, mac, temp["data"], 0, type, 0, true);
			}
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "newDevRsp";
	return CODE_OK;
}

// TODO: delete device from room, group, scene,...
int Gateway::OnDeleteDevice(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("device") && reqValue["device"].isArray())
	{
		Json::Value successList;
		Json::Value failedList;
		Json::Value devicesValue = reqValue["device"];
		for (auto &deviceValue : devicesValue)
		{
			if (deviceValue.isString())
			{
				string deviceId = deviceValue.asString();
				Device *device = getDeviceFromId(deviceId);
				if (device)
				{
					if (bleProtocol)
						bleProtocol->ResetDev(device->GetAddr());

					delDevice(device);
					successList.append(deviceId);
				}
				else
				{
					LOGD("deviceId %s dose not exist", deviceId.c_str());
					failedList.append(deviceId);
				}
			}
		}
		respValue["data"]["code"] = 0;
		respValue["data"]["success"] = successList;
		respValue["data"]["failed"] = failedList;
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "delDevRsp";
	return CODE_OK;
}

int Gateway::OnAddFavoriteDev(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("devlist") && reqValue["devlist"].isArray())
	{
		Json::Value devs = reqValue["devlist"];
		for (auto &temp : devs)
		{
			if (temp.isString())
			{
				string temp_dev = temp.asString();
				Device *dev = getDeviceFromId(temp_dev);
				if (dev)
				{
					for (const auto &[id, device] : deviceList)
					{
						if (id == temp_dev)
						{
							device->SetIsFavorite(true);
							database->DeviceUpdateFavorite(device);
						}
					}
				}
			}
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "addFavoriteDevRsp";
	return CODE_OK;
}

int Gateway::OnDelFavoriteDev(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("devlist") && reqValue["devlist"].isArray())
	{
		Json::Value devs = reqValue["devlist"];
		for (auto &temp : devs)
		{
			if (temp.isString())
			{
				string temp_dev = temp.asString();
				Device *dev = getDeviceFromId(temp_dev);
				if (dev)
				{
					for (const auto &[id, device] : deviceList)
					{
						if (id == temp_dev)
						{
							device->SetIsFavorite(false);
							database->DeviceUpdateFavorite(device);
						}
					}
				}
			}
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "delFavoriteDevRsp";
	return CODE_OK;
}

int Gateway::OnGetFavoriteDev(Json::Value &reqValue, Json::Value &respValue)
{
	Json::Value list;
	for (const auto &[id, device] : deviceList)
	{
		if (device->GetIsFavorite())
		{
			list.append(id);
		}
	}
	respValue["data"]["devices"] = list;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getFavoriteDevRsp";
	return CODE_OK;
}

int Gateway::OnUpdateDeviceName(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("name") && reqValue["name"].isString() && reqValue.isMember("id") && reqValue["id"].isString())
	{
		string id = reqValue["id"].asString();
		string name = reqValue["name"].asString();
		Device *device = getDeviceFromId(id);
		if (device)
		{
			device->SetName(name);
			database->DeviceUpdate(device);
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "updateDeviceNameRsp";
	return CODE_OK;
}
