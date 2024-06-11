#include "Gateway.h"
#include "Log.h"
#include "Db.h"
#include "DeviceBleSeftPowerRemote.h"

void Gateway::InitMqttMessageDevice()
{
	OnDeviceRpcCmdCallbackRegister("setAttribute", "device.onoff", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));

	// OnDeviceRpcCallbackRegister("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getDevStt", bind(&Gateway::OnGetDeviceStatus, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getAllDevStt", bind(&Gateway::OnGetAllDeviceStatus, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getDevList", bind(&Gateway::OnGetDeviceList, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getCamListInRoom", bind(&Gateway::OnGetCamList, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getAllCam", bind(&Gateway::OnGetAllCam, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("newDev", bind(&Gateway::OnNewDevice, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delDev", bind(&Gateway::OnDeleteDevice, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("addFavoriteDev", bind(&Gateway::OnAddFavoriteDev, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delFavoriteDev", bind(&Gateway::OnDelFavoriteDev, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getFavoriteDev", bind(&Gateway::OnGetFavoriteDev, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("updateDeviceName", bind(&Gateway::OnUpdateDeviceName, this, placeholders::_1, placeholders::_2));

	// OnDeviceRpcCallbackRegister("createSwitchLink", bind(&Gateway::OnCreateSwitchLink, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("addBtToSwitchLink", bind(&Gateway::OnAddBtToSwitchLink, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delBtFromSwitchLink", bind(&Gateway::OnDelBtFromSwitchLink, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delSwitchLink", bind(&Gateway::OnDelSwitchLink, this, placeholders::_1, placeholders::_2));

	// OnLocalCallbackRegister("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getDevStt", bind(&Gateway::OnGetDeviceStatus, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getAllDevStt", bind(&Gateway::OnGetAllDeviceStatus, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getDevList", bind(&Gateway::OnGetDeviceList, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getCamListInRoom", bind(&Gateway::OnGetCamList, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getAllCam", bind(&Gateway::OnGetAllCam, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("newDev", bind(&Gateway::OnNewDevice, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delDev", bind(&Gateway::OnDeleteDevice, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("addFavoriteDev", bind(&Gateway::OnAddFavoriteDev, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delFavoriteDev", bind(&Gateway::OnDelFavoriteDev, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getFavoriteDev", bind(&Gateway::OnGetFavoriteDev, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("updateDeviceName", bind(&Gateway::OnUpdateDeviceName, this, placeholders::_1, placeholders::_2));

	// OnLocalCallbackRegister("createSwitchLink", bind(&Gateway::OnCreateSwitchLink, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("addBtToSwitchLink", bind(&Gateway::OnAddBtToSwitchLink, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delBtFromSwitchLink", bind(&Gateway::OnDelBtFromSwitchLink, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delSwitchLink", bind(&Gateway::OnDelSwitchLink, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnControlDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlDevice");
	if (reqValue.isMember("arguments") && reqValue["arguments"].isObject())
	{
		Json::Value &argumentsValue = reqValue["arguments"];
		if (argumentsValue.isMember("mac") && argumentsValue["mac"].isString() &&
			argumentsValue.isMember("ep") && argumentsValue["ep"].isString() &&
			argumentsValue.isMember("value") && argumentsValue["value"].isObject())
		{
			string mac = argumentsValue["mac"].asString();
			string ep = argumentsValue["ep"].asString();
			Json::Value &valueValue = argumentsValue["value"];
			Device *device = getDeviceFromMac(mac);
			if (device)
			{
				int rs = device->Do(valueValue);
			}
			else
			{
				LOGW("Device mac %s not found", mac.c_str());
			}
		}
	}
	else
	{
		LOGW("OnControlDevice %s format error", reqValue.toString().c_str());
	}
	return CODE_NOT_RESPONSE;
}

int Gateway::OnControlAllDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlAllDevice");
	if (bleProtocol)
	{
		if (reqValue.isMember(KEY_ATTRIBUTE_ONOFF) && reqValue[KEY_ATTRIBUTE_ONOFF].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_ONOFF].asInt();
			bleProtocol->SetOnOffLight(0xFFFF, value, TRANSITION_DEFAULT, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_DIM) && reqValue[KEY_ATTRIBUTE_DIM].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_DIM].asInt();
			uint16_t dim = (value * 65535) / 100;
			bleProtocol->SetDimmingLight(0xFFFF, dim, TRANSITION_DEFAULT, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_CCT) && reqValue[KEY_ATTRIBUTE_CCT].isInt())
		{
			int value = reqValue[KEY_ATTRIBUTE_CCT].asInt();
			uint16_t cct = (value * 192) + 800;
			bleProtocol->SetCctLight(0xFFFF, cct, TRANSITION_DEFAULT, true);
		}
		if (reqValue.isMember(KEY_ATTRIBUTE_HUE) && reqValue[KEY_ATTRIBUTE_HUE].isInt() &&
			reqValue.isMember(KEY_ATTRIBUTE_SATURATION) && reqValue[KEY_ATTRIBUTE_SATURATION].isInt() &&
			reqValue.isMember(KEY_ATTRIBUTE_LUMINANCE) && reqValue[KEY_ATTRIBUTE_LUMINANCE].isInt())
		{
			int h = reqValue[KEY_ATTRIBUTE_HUE].asInt();
			int s = reqValue[KEY_ATTRIBUTE_SATURATION].asInt();
			int l = reqValue[KEY_ATTRIBUTE_LUMINANCE].asInt();
			bleProtocol->SetHSLLight(0xFFFF, h, s, l, TRANSITION_DEFAULT, true);
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
		Json::Value devicesValueRsp = Json::arrayValue;
		Json::Value devicesValue = reqValue["devices"];
		for (auto &deviceValue : devicesValue)
		{
			if (deviceValue.isString())
			{
				string deviceId = deviceValue.asString();
				Device *device = getDeviceFromId(deviceId);
				if (device)
				{
					Json::Value deviceValue = Json::objectValue;
					if (device->GetType() == BLE_PM_SENSOR)
					{
						if (bleProtocol)
							bleProtocol->UpdateStatusSensorsPm(device->GetAddr());
					}
					else
					{
						deviceValue["id"] = device->GetId();
						Json::Value deviceAttbute = Json::objectValue;
						device->BuildTelemetryValue(deviceAttbute);
						deviceValue["data"] = deviceAttbute;
						devicesValueRsp.append(deviceValue);
					}
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
	Json::Value devicesValueRsp = Json::arrayValue;
	deviceListMtx.lock();
	for (const auto &[id, device] : deviceList)
	{
		Json::Value deviceValue;
		deviceValue["id"] = device->GetId();
		Json::Value deviceAttbute = Json::objectValue;
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
	Json::Value devicesValueRsp = Json::arrayValue;
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
	Json::Value roomData = Json::arrayValue;
	for (const auto &[id, room] : roomList)
	{
		Json::Value temp_devicesList;
		temp_devicesList["camList"] = Json::arrayValue;
		for (unsigned int i = 0; i < room->deviceList.size(); i++)
		{
			DeviceInGroup *deviceInRoom = room->deviceList[i];
			int type = deviceInRoom->device->GetType();
			if (type / 10000 == 6)
			{
				Json::Value deviceValue;
				deviceValue["id"] = deviceInRoom->device->GetId();
				deviceValue["mac"] = deviceInRoom->device->GetMac();
				deviceValue["name"] = deviceInRoom->device->GetName();
				deviceValue["data"] = deviceInRoom->device->GetData();
				temp_devicesList["camList"].append(deviceValue);
			}
		}
		temp_devicesList["id"] = id;
		roomData.append(temp_devicesList);
	}
	respValue["data"]["room"] = roomData;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getCamListInRoomRsp";
	return CODE_OK;
}

int Gateway::OnGetAllCam(Json::Value &reqValue, Json::Value &respValue)
{
	LOGE("OnGetAllCam");
	Json::Value camData = Json::arrayValue;
	for (const auto &[id, device] : deviceList)
	{
		int type = device->GetType();
		if (type / 10000 == 6)
		{
			Json::Value deviceValue;
			deviceValue["id"] = device->GetId();
			deviceValue["mac"] = device->GetMac();
			deviceValue["name"] = device->GetName();
			deviceValue["data"] = device->GetData();
			camData.append(deviceValue);
		}
	}
	respValue["data"]["devices"] = camData;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getAllCamRsp";
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
		Json::Value successList = Json::arrayValue;
		Json::Value failedList = Json::arrayValue;
		Json::Value devicesValue = reqValue["device"];
		for (auto &deviceValue : devicesValue)
		{
			if (deviceValue.isString())
			{
				string deviceId = deviceValue.asString();
				Device *device = getDeviceFromId(deviceId);
				if (device)
				{
					if (device->GetType() == BLE_SEFTPOWER_REMOTE_1 || device->GetType() == BLE_SEFTPOWER_REMOTE_2 || device->GetType() == BLE_SEFTPOWER_REMOTE_3)
					{
						DeviceBleSeftPowerRemote *deviceBleSeftPowerRemote = dynamic_cast<DeviceBleSeftPowerRemote *>(device);
						if (deviceBleSeftPowerRemote)
						{
							Device *parent = deviceBleSeftPowerRemote->GetParent();
							if (parent)
							{
								database->DeviceBleChildDel(deviceBleSeftPowerRemote, parent);
								bleProtocol->ResetSeftPowerRemote(parent->GetAddr(), deviceBleSeftPowerRemote->GetAddr());
							}
							else
								LOGW("parent device null");
						}
					}
					else if (bleProtocol)
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
		pushMsgHcCoreToHcApp("delDev", "", "", successList, "");
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
	Json::Value list = Json::arrayValue;
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
	Json::Value dataPushToHcApp;
	dataPushToHcApp["cmd"] = "updateDeviceName";
	dataPushToHcApp["data"] = reqValue;
	PublishToLocalMessage(dataPushToHcApp);
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "updateDeviceNameRsp";
	return CODE_OK;
}

static int indexBt(string bt)
{
	string a[] = {"bt", "bt2", "bt3", "bt4", "bt5", "bt6"};
	for (int i = 0; i < 6; i++)
	{
		if (a[i] == bt)
		{
			return i;
		}
	}
	return -1;
}

int Gateway::OnCreateSwitchLink(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnCreateSwitchLink");
	respValue["cmd"] = "createSwitchLinkRsp";
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string groupId = reqValue["id"].asString();
		Group *group = getGroupFromId(groupId);
		if (!group)
		{
			group = new Group(groupId, getNextGroupAddr(), groupId);
			if (group)
				AddNewGroup(group, true);
		}

		if (group)
		{
			Json::Value listBtSuccess = Json::arrayValue;
			Json::Value listBtFailure = Json::arrayValue;
			Json::Value listSuccess = Json::arrayValue;
			Json::Value listFailure = Json::arrayValue;
			if (reqValue.isMember("lstBt") && reqValue["lstBt"].isArray())
			{
				Json::Value lstBt = reqValue["lstBt"];
				for (auto &btn : lstBt)
				{
					if (btn.isObject() && btn.isMember("id") && btn["id"].isString() && btn.isMember("bt") && btn["bt"].isArray())
					{
						string devId = btn["id"].asString();
						Device *device = getDeviceFromId(devId);
						if (device)
						{
							if (device->GetType() == BLE_SWITCH_1 ||
								device->GetType() == BLE_SWITCH_2 ||
								device->GetType() == BLE_SWITCH_3 ||
								device->GetType() == BLE_SWITCH_4 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_1 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_2 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_3 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_4 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_1_V2 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_2_V2 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_3_V2 ||
								device->GetType() == BLE_WIFI_SWITCH_ELECTRICAL_1 ||
								device->GetType() == BLE_WIFI_SWITCH_ELECTRICAL_2 ||
								device->GetType() == BLE_WIFI_SWITCH_ELECTRICAL_3 ||
								device->GetType() == BLE_SWITCH_RGB_1 ||
								device->GetType() == BLE_SWITCH_RGB_2 ||
								device->GetType() == BLE_SWITCH_RGB_3 ||
								device->GetType() == BLE_SWITCH_RGB_4 ||
								device->GetType() == BLE_SWITCH_RGB_1_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_2_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_3_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_4_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_1_V2 ||
								device->GetType() == BLE_SWITCH_RGB_1_SQUARE_V2 ||
								device->GetType() == BLE_SWITCH_RGB_2_V2 ||
								device->GetType() == BLE_SWITCH_RGB_2_SQUARE_V2 ||
								device->GetType() == BLE_SWITCH_RGB_3_V2 ||
								device->GetType() == BLE_SWITCH_RGB_3_SQUARE_V2 ||
								device->GetType() == BLE_SWITCH_RGB_4_V2 ||
								device->GetType() == BLE_SWITCH_RGB_4_SQUARE_V2 ||
								device->GetType() == BLE_WIFI_SWITCH_1 ||
								device->GetType() == BLE_WIFI_SWITCH_2 ||
								device->GetType() == BLE_WIFI_SWITCH_3 ||
								device->GetType() == BLE_WIFI_SWITCH_4)
							{
								for (auto &bt : btn["bt"])
								{
									if (bt.isString())
									{
										int idxBt = indexBt(bt.asString());
										if ((idxBt >= 0) && (idxBt < 6))
										{
											if (group->AddDevice(device, device->GetAddr() + idxBt, true, true) == CODE_OK)
											{
												if (bleProtocol->SetIdCombine(device->GetAddr() + idxBt, group->GetAddr() + 49152) == CODE_OK)
												{
													listBtSuccess.append(bt.asString());
												}
												else
												{
													listBtFailure.append(bt.asString());
												}
											}
											else
											{
												listBtFailure.append(bt.asString());
											}
										}
									}
								}
								if (listBtSuccess.size() > 0)
								{
									Json::Value success;
									success["id"] = devId;
									success["bt"] = listBtSuccess;
									listSuccess.append(success);
								}
								if (listBtFailure.size() > 0)
								{
									Json::Value failed;
									failed["id"] = devId;
									failed["bt"] = listBtFailure;
									listFailure.append(failed);
								}
								respValue["data"]["code"] = CODE_OK;
							}
							else
							{
								respValue["data"]["code"] = CODE_ERROR;
							}
						}
						else
						{
							respValue["data"]["code"] = CODE_NOT_FOUND_DEVICE;
						}
						respValue["data"]["success"] = listSuccess;
						respValue["data"]["failed"] = listFailure;
					}
					else
					{
						respValue["data"]["code"] = CODE_FORMAT_ERROR;
					}
				}
			}
			else
			{
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
			}
		}
		else
		{
			respValue["data"]["code"] = CODE_MEMORY_ERROR;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	return CODE_OK;
}

int Gateway::OnAddBtToSwitchLink(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnAddBtSwitchLink");
	respValue["cmd"] = "addBtToSwitchLinkRsp";
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string groupId = reqValue["id"].asString();
		Group *group = getGroupFromId(groupId);
		if (!group)
		{
			group = new Group(groupId, getNextGroupAddr(), groupId);
			if (group)
				AddNewGroup(group, true);
		}

		if (group)
		{
			Json::Value listBtSuccess = Json::arrayValue;
			Json::Value listBtFailure = Json::arrayValue;
			Json::Value listSuccess = Json::arrayValue;
			Json::Value listFailure = Json::arrayValue;
			if (reqValue.isMember("lstBt") && reqValue["lstBt"].isArray())
			{
				Json::Value lstBt = reqValue["lstBt"];
				for (auto &btn : lstBt)
				{
					if (btn.isObject() && btn.isMember("id") && btn["id"].isString() && btn.isMember("bt") && btn["bt"].isArray())
					{
						string devId = btn["id"].asString();
						Device *device = getDeviceFromId(devId);
						if (device)
						{
							if (device->GetType() == BLE_SWITCH_1 ||
								device->GetType() == BLE_SWITCH_2 ||
								device->GetType() == BLE_SWITCH_3 ||
								device->GetType() == BLE_SWITCH_4 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_1 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_2 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_3 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_4 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_1_V2 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_2_V2 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_3_V2 ||
								device->GetType() == BLE_WIFI_SWITCH_ELECTRICAL_1 ||
								device->GetType() == BLE_WIFI_SWITCH_ELECTRICAL_2 ||
								device->GetType() == BLE_WIFI_SWITCH_ELECTRICAL_3 ||
								device->GetType() == BLE_SWITCH_RGB_1 ||
								device->GetType() == BLE_SWITCH_RGB_2 ||
								device->GetType() == BLE_SWITCH_RGB_3 ||
								device->GetType() == BLE_SWITCH_RGB_4 ||
								device->GetType() == BLE_SWITCH_RGB_1_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_2_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_3_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_4_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_1_V2 ||
								device->GetType() == BLE_SWITCH_RGB_1_SQUARE_V2 ||
								device->GetType() == BLE_SWITCH_RGB_2_V2 ||
								device->GetType() == BLE_SWITCH_RGB_2_SQUARE_V2 ||
								device->GetType() == BLE_SWITCH_RGB_3_V2 ||
								device->GetType() == BLE_SWITCH_RGB_3_SQUARE_V2 ||
								device->GetType() == BLE_SWITCH_RGB_4_V2 ||
								device->GetType() == BLE_SWITCH_RGB_4_SQUARE_V2 ||
								device->GetType() == BLE_WIFI_SWITCH_1 ||
								device->GetType() == BLE_WIFI_SWITCH_2 ||
								device->GetType() == BLE_WIFI_SWITCH_3 ||
								device->GetType() == BLE_WIFI_SWITCH_4)
							{
								for (auto &bt : btn["bt"])
								{
									if (bt.isString())
									{
										int idxBt = indexBt(bt.asString());
										if ((idxBt >= 0) && (idxBt < 6))
										{
											if (group->AddDevice(device, device->GetAddr() + idxBt, true, true) == CODE_OK)
											{
												if (bleProtocol->SetIdCombine(device->GetAddr() + idxBt, group->GetAddr() + 49152) == CODE_OK)
												{
													listBtSuccess.append(bt.asString());
												}
												else
												{
													listBtFailure.append(bt.asString());
												}
											}
											else
											{
												listBtFailure.append(bt.asString());
											}
										}
									}
								}
								if (listBtSuccess.size() > 0)
								{
									Json::Value success;
									success["id"] = devId;
									success["bt"] = listBtSuccess;
									listSuccess.append(success);
								}
								if (listBtFailure.size() > 0)
								{
									Json::Value failed;
									failed["id"] = devId;
									failed["bt"] = listBtFailure;
									listFailure.append(failed);
								}
								respValue["data"]["code"] = CODE_OK;
							}
							else
							{
								respValue["data"]["code"] = CODE_ERROR;
							}
						}
						else
						{
							respValue["data"]["code"] = CODE_NOT_FOUND_DEVICE;
						}
						respValue["data"]["success"] = listSuccess;
						respValue["data"]["failed"] = listFailure;
					}
					else
					{
						respValue["data"]["code"] = CODE_FORMAT_ERROR;
					}
				}
			}
			else
			{
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
			}
		}
		else
		{
			respValue["data"]["code"] = CODE_MEMORY_ERROR;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	return CODE_OK;
}

int Gateway::OnDelBtFromSwitchLink(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("DelBtSwitchLink");
	respValue["cmd"] = "delBtFromSwitchLinkRsp";
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string groupId = reqValue["id"].asString();
		Group *group = getGroupFromId(groupId);
		if (group)
		{
			Json::Value listBtSuccess = Json::arrayValue;
			Json::Value listBtFailure = Json::arrayValue;
			Json::Value listSuccess = Json::arrayValue;
			Json::Value listFailure = Json::arrayValue;
			if (reqValue.isMember("lstBt") && reqValue["lstBt"].isArray())
			{
				Json::Value lstBt = reqValue["lstBt"];
				for (auto &btn : lstBt)
				{
					if (btn.isObject() && btn.isMember("id") && btn["id"].isString() && btn.isMember("bt") && btn["bt"].isArray())
					{
						string devId = btn["id"].asString();
						Device *device = getDeviceFromId(devId);
						if (device)
						{
							if (device->GetType() == BLE_SWITCH_1 ||
								device->GetType() == BLE_SWITCH_2 ||
								device->GetType() == BLE_SWITCH_3 ||
								device->GetType() == BLE_SWITCH_4 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_1 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_2 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_3 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_4 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_1_V2 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_2_V2 ||
								device->GetType() == BLE_SWITCH_ELECTRICAL_3_V2 ||
								device->GetType() == BLE_WIFI_SWITCH_ELECTRICAL_1 ||
								device->GetType() == BLE_WIFI_SWITCH_ELECTRICAL_2 ||
								device->GetType() == BLE_WIFI_SWITCH_ELECTRICAL_3 ||
								device->GetType() == BLE_SWITCH_RGB_1 ||
								device->GetType() == BLE_SWITCH_RGB_2 ||
								device->GetType() == BLE_SWITCH_RGB_3 ||
								device->GetType() == BLE_SWITCH_RGB_4 ||
								device->GetType() == BLE_SWITCH_RGB_1_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_2_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_3_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_4_SQUARE ||
								device->GetType() == BLE_SWITCH_RGB_1_V2 ||
								device->GetType() == BLE_SWITCH_RGB_1_SQUARE_V2 ||
								device->GetType() == BLE_SWITCH_RGB_2_V2 ||
								device->GetType() == BLE_SWITCH_RGB_2_SQUARE_V2 ||
								device->GetType() == BLE_SWITCH_RGB_3_V2 ||
								device->GetType() == BLE_SWITCH_RGB_3_SQUARE_V2 ||
								device->GetType() == BLE_SWITCH_RGB_4_V2 ||
								device->GetType() == BLE_SWITCH_RGB_4_SQUARE_V2 ||
								device->GetType() == BLE_WIFI_SWITCH_1 ||
								device->GetType() == BLE_WIFI_SWITCH_2 ||
								device->GetType() == BLE_WIFI_SWITCH_3 ||
								device->GetType() == BLE_WIFI_SWITCH_4)
							{
								for (auto &bt : btn["bt"])
								{
									if (bt.isString())
									{
										int idxBt = indexBt(bt.asString());
										if ((idxBt >= 0) && (idxBt < 6))
										{
											if (group->DelDevice(device, device->GetAddr() + idxBt, true, true) == CODE_OK)
											{
												if (bleProtocol->SetIdCombine(device->GetAddr() + idxBt, 0) == CODE_OK)
												{
													listBtSuccess.append(bt.asString());
												}
												else
												{
													listBtFailure.append(bt.asString());
												}
											}
											else
											{
												listBtFailure.append(bt.asString());
											}
										}
									}
								}
								if (listBtSuccess.size() > 0)
								{
									Json::Value success;
									success["id"] = devId;
									success["bt"] = listBtSuccess;
									listSuccess.append(success);
								}
								if (listBtFailure.size() > 0)
								{
									Json::Value failed;
									failed["id"] = devId;
									failed["bt"] = listBtFailure;
									listFailure.append(failed);
								}
								respValue["data"]["code"] = CODE_OK;
							}
							else
							{
								respValue["data"]["code"] = CODE_ERROR;
							}
						}
						else
						{
							respValue["data"]["code"] = CODE_NOT_FOUND_DEVICE;
						}
						respValue["data"]["success"] = listSuccess;
						respValue["data"]["failed"] = listFailure;
					}
					else
					{
						respValue["data"]["code"] = CODE_FORMAT_ERROR;
					}
				}
			}
			else
			{
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
			}
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_GROUP;
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	return CODE_OK;
}

int Gateway::OnDelSwitchLink(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("Del Switch Link");
	respValue["cmd"] = "delSwitchLinkRsp";
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string groupId = reqValue["id"].asString();
		Group *group = getGroupFromId(groupId);
		if (group)
		{
			Json::Value listBtSuccess = Json::arrayValue;
			Json::Value listBtFailure = Json::arrayValue;
			Json::Value listSuccess = Json::arrayValue;
			Json::Value listFailure = Json::arrayValue;
			vector<DeviceInGroup *> listDev = group->deviceList;
			for (auto &dev : listDev)
			{
				if (group->DelDevice(dev->device, dev->epId, true, true) == CODE_OK)
				{
					if (bleProtocol->SetIdCombine(dev->epId, 0) == CODE_OK)
					{
						respValue["data"]["coode"] = CODE_OK;
					}
					else
					{
						respValue["data"]["coode"] = CODE_ERROR;
					}
				}
				else
				{
					respValue["data"]["coode"] = CODE_ERROR;
				}
			}
			delGroup(group);
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_GROUP;
		}
	}
	return CODE_OK;
}
