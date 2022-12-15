#ifdef CONFIG_THINGSBOARD

#include "Gateway.h"
#include <Log.h>
#include <unistd.h>
#include <stdio.h>
#include <algorithm>
#include <json.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <thread>
#include "Db.h"
#include "Util.h"
#include "Base64.h"

#include "SceneInputTimer.h"
#include "SceneOutputGroup.h"
#include "SceneOutputDevice.h"
#include "SceneOutputRelay.h"

#ifdef CONFIG_ENABLE_BLE
#include "BleProtocol.h"
#include "DeviceBleDownLightSmt.h"
#include "DeviceBleSwitch4.h"
#include "DeviceBleDCSceneContact.h"
#include "DeviceBleTempHumSensor.h"
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#include "DeviceZigbeeOnoff.h"
#include "DeviceZigbeeTelinkOnoff.h"
#endif

#ifdef CONFIG_ENABLE_LORA
#include "LoraProtocol.h"
#include "DeviceLoraCo2Sensor.h"
#include "DeviceLoraLightSensor.h"
#include "DeviceLoraSoilMoisSensor.h"
#include "DeviceLoraECSensor.h"
#include "DeviceLoraTempHumSensor.h"
#endif

#ifdef CONFIG_ENABLE_MODBUS
#include "ModbusProtocol.h"
#include "DeviceModbusES_SM_TH_01.h"
#endif

#ifdef CONFIG_ENABLE_MCU
#include "BATProtocol.h"
#endif

Gateway *gateway = NULL;

Gateway::Gateway(string server_address, int server_port, string token, string username, string password, int keepalive) : CloudProtocol(server_address, server_port, token, username, password, keepalive)
{
}

void Gateway::init()
{
	CloudProtocol::init();

	LOGI("DeviceRead");
	database->DeviceRead();
#ifdef CONFIG_ENABLE_MODBUS
	database->ModbusDeviceRead();
	database->ModbusParameterRead();
#endif
	database->GroupRead();
	database->DeviceInGroupRead();
	database->SceneRead();

#ifdef CONFIG_ENABLE_BLE
	OnDeviceRPCCallbackRegister("BleStartScan", bind(&Gateway::OnRPCBleStartScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("BleStopScan", bind(&Gateway::OnRPCBleStopScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("BleResetFactory", bind(&Gateway::OnRPCBleResetFactory, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("BleAddDevice", bind(&Gateway::OnRPCBleAddDevice, this, placeholders::_1, placeholders::_2));
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
	OnDeviceRPCCallbackRegister("ZigbeeStartScan", bind(&Gateway::OnRPCZigbeeStartScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("ZigbeeStopScan", bind(&Gateway::OnRPCZigbeeStopScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("ZigbeeResetFactory", bind(&Gateway::OnRPCZigbeeResetFactory, this, placeholders::_1, placeholders::_2));
#endif
#ifdef CONFIG_ENABLE_LORA
	OnDeviceRPCCallbackRegister("LoraStartScan", bind(&Gateway::OnRPCLoraStartScan, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("LoraStopScan", bind(&Gateway::OnRPCLoraStopScan, this, placeholders::_1, placeholders::_2));
#endif
#ifdef CONFIG_ENABLE_MCU
	OnDeviceRPCCallbackRegister("ControlRelay", bind(&Gateway::OnRPCControlRelay, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("ControlAllRelay", bind(&Gateway::OnRPCControlAllRelay, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("GetRelayState", bind(&Gateway::OnRPCGetRelayState, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("SetDimming", bind(&Gateway::OnRPCSetDimming, this, placeholders::_1, placeholders::_2));
#endif
	OnDeviceRPCCallbackRegister("AddGroup", bind(&Gateway::OnRPCAddGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("UpdateGroup", bind(&Gateway::OnRPCUpdateGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DelGroup", bind(&Gateway::OnRPCDelGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("AddDeviceToGroup", bind(&Gateway::OnRPCAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DelDeviceFromGroup", bind(&Gateway::OnRPCDelDeviceFromGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("AddDevice", bind(&Gateway::OnRPCAddDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DelAllDevice", bind(&Gateway::OnRPCDelAllDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("GetScanDevice", bind(&Gateway::OnRPCGetScanDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("AddScene", bind(&Gateway::OnRPCAddScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("DeleteScene", bind(&Gateway::OnRPCDeleteScene, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("ControlDevice", bind(&Gateway::OnRPCControlDevice, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("ControlGroup", bind(&Gateway::OnRPCControlGroup, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("UpdateAllTelemetry", bind(&Gateway::OnRPCUpdateAllTelemetry, this, placeholders::_1, placeholders::_2));
	OnDeviceRPCCallbackRegister("SSHRemote", bind(&Gateway::OnRPCSSHRemote, this, placeholders::_1, placeholders::_2));

	auto connectFunc = bind(&Mqtt::Connect, this, placeholders::_1);
	thread connectThread(connectFunc, 10);
	connectThread.detach();
}

void Gateway::OnConnect(bool isConnected, bool isReconnect)
{
	LOGI("Connected: %d", isConnected);
	if (isConnected && !isReconnect)
	{
		for (const auto &[id, device] : deviceList)
		{
			device->PushAttributes();
		}
	}
}

#ifdef CONFIG_ENABLE_BLE
int Gateway::OnRPCBleStartScan(Json::Value &reqValue, Json::Value &respValue)
{
	scanDeviceList.clear();
	bleProtocol->StartScan();
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCBleStopScan(Json::Value &reqValue, Json::Value &respValue)
{
	bleProtocol->StopScan();
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCBleResetFactory(Json::Value &reqValue, Json::Value &respValue)
{
	bleProtocol->ResetFactory();
	bleProtocol->init();
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCBleAddDevice(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if ( // paramsValue.isMember("id") && paramsValue["id"].isString() &&
				paramsValue.isMember("name") && paramsValue["name"].isString() &&
				paramsValue.isMember("mac") && paramsValue["mac"].isString() &&
				paramsValue.isMember("type") && paramsValue["type"].isInt64())
		{
			// string deviceId = ""; // paramsValue["id"].asString();
			// string name = paramsValue["name"].asString();
			// string mac = paramsValue["mac"].asString();
			// uint32_t type = paramsValue["type"].asInt64();
			// int rs = bleProtocol->AddDevice(deviceId, name, mac, type);
			// respValue["code"] = rs;
			// return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
int Gateway::OnRPCZigbeeStartScan(Json::Value &reqValue, Json::Value &respValue)
{
	scanDeviceList.clear();
	zigbeeProtocol->PermitJoin(60);
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCZigbeeStopScan(Json::Value &reqValue, Json::Value &respValue)
{
	zigbeeProtocol->PermitJoin(0);
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCZigbeeResetFactory(Json::Value &reqValue, Json::Value &respValue)
{
	zigbeeProtocol->ResetFactory();
	zigbeeProtocol->CommissionFormation();
	respValue["code"] = 0;
	return 0;
}
#endif

#ifdef CONFIG_ENABLE_LORA
int Gateway::OnRPCLoraStartScan(Json::Value &reqValue, Json::Value &respValue)
{
	loraProtocol->StartScan();
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCLoraStopScan(Json::Value &reqValue, Json::Value &respValue)
{
	loraProtocol->StopScan();
	respValue["code"] = 0;
	return 0;
}
#endif

#ifdef CONFIG_ENABLE_MCU
int Gateway::OnRPCControlRelay(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("relay") && paramsValue["relay"].isInt() && paramsValue.isMember("value") && paramsValue["value"].isBool())
		{
			int relay = paramsValue["relay"].asInt();
			bool value = paramsValue["value"].asBool();
			uint8_t relayState = 0;
			int rs = batProtocol->ControlRelay(relay, value, relayState);
			if (rs == 0)
			{
				LOGD("relayState: 0x%02X", relayState);
				respValue["relay_state"] = relayState;
				respValue["code"] = 0;
			}
			else
			{
				LOGW("ControlRelay err: %d", rs);
				respValue["relay_state"] = -1;
				respValue["code"] = 1;
			}
			PushRelayState(relayState);
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCControlAllRelay(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("all") && paramsValue["all"].isInt())
		{
			int all = paramsValue["all"].asInt();
			uint8_t relayState = 0;
			int rs = batProtocol->ControlAllRelay(all, relayState);
			if (rs == 0)
			{
				LOGD("relayState: 0x%02X", relayState);
				respValue["relay_state"] = relayState;
				respValue["code"] = 0;
			}
			else
			{
				LOGW("ControlRelay err: %d", rs);
				respValue["relay_state"] = -1;
				respValue["code"] = 1;
			}
			PushRelayState(relayState);
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCGetRelayState(Json::Value &reqValue, Json::Value &respValue)
{
	uint8_t relayState = 0;
	if (batProtocol->RequestRelayState(relayState) == 0)
	{
		LOGD("relayState: 0x%02X", relayState);
		respValue["relay_state"] = relayState;
		respValue["code"] = 0;
		return 0;
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCSetDimming(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("id") && paramsValue["id"].isInt() && paramsValue.isMember("value") && paramsValue["value"].isInt())
		{
			int dim_id = paramsValue["id"].asInt();
			int value = paramsValue["value"].asInt();
			uint8_t relayState = 0;
			if (batProtocol->SetDimming(dim_id, value) == 0)
			{
				LOGD("SetDimming: 0x%02X", relayState);
				respValue["id"] = dim_id;
				respValue["value"] = relayState;
				respValue["code"] = 0;
				return 0;
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}
#endif

int Gateway::OnRPCAddScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddScene");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		Scene *scene = AddScene(paramsValue, true, true);
		if (scene)
		{
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCDeleteScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDeleteScene");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("id") && paramsValue["id"].isInt())
		{
			int sceneId = paramsValue["id"].asInt();
			LOGI("Delete Scene id: %d", sceneId);
			delete sceneList[sceneId];
			sceneList.erase(sceneList.find(sceneId));
			database->SceneDel(sceneId);
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCAddGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddGroup");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("id") && paramsValue["id"].isInt() &&
				paramsValue.isMember("name") && paramsValue["name"].isString())
		{
			int groupId = paramsValue["id"].asInt();
			string name = paramsValue["name"].asString();
			Group *group = new Group(groupId, name);
			if (group)
			{
				group = AddNewGroup(group, true, true);
				if (group)
				{
					respValue["code"] = 0;
					return 0;
				}
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCUpdateGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCUpdateGroup");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("id") && paramsValue["id"].isInt() &&
				paramsValue.isMember("name") && paramsValue["name"].isString())
		{
			int groupId = paramsValue["id"].asInt();
			string name = paramsValue["name"].asString();
			Group *group = getGroup(groupId);
			if (group)
			{
				group->SetName(name);
				database->GroupUpdate(group);
				respValue["code"] = 0;
				return 0;
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCDelGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDelGroup");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("id") && paramsValue["id"].isInt())
		{
			int groupId = paramsValue["id"].asInt();
			LOGI("Delete Scene id: %d", groupId);
			delete groupList[groupId];
			groupList.erase(groupList.find(groupId));
			database->GroupDel(groupId);
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCAddDeviceToGroup");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("groupId") && paramsValue["groupId"].isInt() &&
				paramsValue.isMember("deviceMac") && paramsValue["deviceMac"].isString() &&
				paramsValue.isMember("epId") && paramsValue["epId"].isInt())
		{
			int groupId = paramsValue["groupId"].asInt();
			string deviceMac = paramsValue["deviceMac"].asString();
			int epId = paramsValue["epId"].asInt();
			Group *group = getGroup(groupId);
			Device *device = getDevice(deviceMac);
			if (group && device)
			{
				if (group->AddDevice(device, epId))
				{
					database->DeviceInGroupAdd(group, device, epId);
					respValue["code"] = 0;
					return 0;
				}
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCDelDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDelDeviceFromGroup");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("groupId") && paramsValue["groupId"].isInt() &&
				paramsValue.isMember("deviceMac") && paramsValue["deviceMac"].isString() &&
				paramsValue.isMember("epId") && paramsValue["epId"].isInt())
		{
			int groupId = paramsValue["groupId"].asInt();
			string deviceMac = paramsValue["deviceMac"].asString();
			int epId = paramsValue["epId"].asInt();
			Group *group = getGroup(groupId);
			Device *device = getDevice(deviceMac);
			if (group && device)
			{
				group->DelDevice(device, epId);
				database->DeviceInGroupDel(group, device, epId);
				respValue["code"] = 0;
				return 0;
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCAddDevice(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("id") && paramsValue["id"].isString() &&
				paramsValue.isMember("name") && paramsValue["name"].isString() &&
				paramsValue.isMember("mac") && paramsValue["mac"].isString() &&
				paramsValue.isMember("addr") && paramsValue["addr"].isInt() &&
				paramsValue.isMember("type") && paramsValue["type"].isInt())
		{
			string deviceId = paramsValue["id"].asString();
			string name = paramsValue["name"].asString();
			string mac = paramsValue["mac"].asString();
			uint32_t addr = paramsValue["addr"].asInt();
			uint32_t type = paramsValue["type"].asInt();
			Device *device = AddNewDevice(deviceId, name, mac, addr, type, true, true);
#ifdef CONFIG_ENABLE_MODBUS
			DeviceModbus *deviceModbus = dynamic_cast<DeviceModbus *>(device);
			if (deviceModbus)
			{
				LOGW("Add modbus device");
				if (paramsValue.isMember("serialPort") && paramsValue["serialPort"].isString() &&
						paramsValue.isMember("baudrate") && paramsValue["baudrate"].isInt() &&
						paramsValue.isMember("scanRate") && paramsValue["scanRate"].isInt() &&
						paramsValue.isMember("timeout") && paramsValue["timeout"].isInt())
				{
					string serialPort = paramsValue["serialPort"].asString();
					uint32_t baudrate = paramsValue["baudrate"].asInt();
					uint32_t scanRate = paramsValue["scanRate"].asInt();
					uint32_t timeout = paramsValue["timeout"].asInt();
					deviceModbus->SetModbusConfig(serialPort, baudrate, scanRate, timeout);
					if (modbusProtocol)
					{
						modbusProtocol->AddDeviceModbus(deviceModbus);
					}
					else
					{
						LOGW("Must init Modbus Protocol first");
					}
				}
				else
				{
					LOGW("Config modbus device message format error");
				}
			}
			else
			{
				LOGW("Cast modbus device false");
			}
#endif
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCDelAllDevice(Json::Value &reqValue, Json::Value &respValue)
{
	database->DeviceDelAll();
#ifdef CONFIG_ENABLE_MODBUS
	database->ModbusDeviceDelAll();
#endif
	deviceList.clear();
#ifdef CONFIG_ENABLE_BLE
	bleProtocol->ResetFactory();
#endif
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCGetScanDevice(Json::Value &reqValue, Json::Value &respValue)
{
	Json::Value scanDeviceValues;
	for (auto &scanDevice : scanDeviceList)
	{
		Json::Value scanDeviceValue;
		scanDeviceValue["name"] = scanDevice->GetName();
		scanDeviceValue["mac"] = scanDevice->GetMac();
		scanDeviceValue["type"] = scanDevice->GetType();
		scanDeviceValue["version"] = scanDevice->GetVersion();
		scanDeviceValue["rssi"] = scanDevice->GetRSSI();
		scanDeviceValues.append(scanDeviceValue);
		// delete scanDevice;
	}
	respValue["devices"] = scanDeviceValues;
	respValue["count"] = scanDeviceList.size();
	respValue["code"] = 0;
	// scanDeviceList.clear();
	Json::Value jsonValue;
	jsonValue["HaveNewDevice"] = false;
	PublishToDeviceTelemetry(jsonValue);
	return 0;
}

int Gateway::OnRPCControlGroup(Json::Value &reqValue, Json::Value &respValue)
{
	int rs = 0;
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("id") && paramsValue["id"].isInt() &&
				paramsValue.isMember("data") && paramsValue["data"].isObject())
		{
			int id = paramsValue["id"].asInt();
			Json::Value dataValue = paramsValue["data"];
			Group *group = gateway->getGroup(id);
			if (group)
			{
				if (group->Do(dataValue))
					rs = 0;
				else
					rs = -1;
			}
		}
	}
	return rs;
}

int Gateway::OnRPCControlDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCControlDevice");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("mac") && paramsValue["mac"].isString() &&
				paramsValue.isMember("data") && paramsValue["data"].isObject())
		{
			string mac = paramsValue["mac"].asString();
			Json::Value dataValue = paramsValue["data"];
			Device *device = getDevice(mac);
			if (device)
			{
				LOGD("Control device %s", device->GetName().c_str());
				device->Do(dataValue);
			}
			respValue["code"] = 0;
			return 0;
		}
	}
	respValue["code"] = -1;
	return -1;
}

int Gateway::OnRPCUpdateAllTelemetry(Json::Value &reqValue, Json::Value &respValue)
{
	Json::Value devicesValue;
	Json::Value deviceData;
	Json::Value dataValue;
	Json::Value values;
	for (const auto &[id, device] : deviceList)
	{
		values = Json::Value::null;
		device->BuildTelemetryValue(values);
		dataValue["values"] = values;
		dataValue["ts"] = to_string(time(NULL)) + "000";
		deviceData.append(dataValue);
		devicesValue[device->GetId()] = deviceData;
	}
	if (!devicesValue.isNull())
		PublishToGatewayTelemetry(devicesValue);
	respValue["code"] = 0;
	return 0;
}

int Gateway::OnRPCSSHRemote(Json::Value &reqValue, Json::Value &respValue)
{
	int rs = 0;
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value paramsValue = reqValue["params"];
		if (paramsValue.isMember("type") && paramsValue["type"].isString() &&
				paramsValue.isMember("key") && paramsValue["key"].isString() &&
				paramsValue.isMember("user") && paramsValue["user"].isString() &&
				paramsValue.isMember("host") && paramsValue["host"].isString() &&
				paramsValue.isMember("serverPort") && paramsValue["serverPort"].isInt() &&
				paramsValue.isMember("forwardPort") && paramsValue["forwardPort"].isInt())
		{
			string key = "";
			string type = paramsValue["type"].asString();
			string user = paramsValue["user"].asString();
			string host = paramsValue["host"].asString();
			uint32_t serverPort = paramsValue["serverPort"].asInt();
			uint32_t forwardPort = paramsValue["forwardPort"].asInt();
			uint32_t localPort = 22;
			if (paramsValue.isMember("localPort") && paramsValue["localPort"].isInt())
			{
				localPort = paramsValue["localPort"].asInt();
			}
			if (type == "base64")
			{
				string keyBase64 = paramsValue["key"].asString();
				string encode = macaron::Base64::Decode(keyBase64, key);
				if (encode != "")
				{
					rs = 1;
				}
			}
			else
			{
				key = paramsValue["key"].asString();
			}

			if (rs == 0)
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
				return 0;
			}
		}
	}
	respValue["code"] = -1;
	return -1;
}

void Gateway::AddDeviceToScanList(Device *scanDevice)
{
	Json::Value jsonValue;
	if (!scanDevice)
	{
		LOGW("scanDevice null");
		return;
	}
	for (auto &scanDeviceLoop : scanDeviceList)
	{
		if (scanDeviceLoop->GetMac() == scanDevice->GetMac())
		{
			scanDeviceLoop->SetRSSI(scanDevice->GetRSSI());
			delete scanDevice;
			jsonValue["HaveNewDevice"] = true;
			PublishToDeviceTelemetry(jsonValue);
			return;
		}
	}
	scanDeviceList.push_back(scanDevice);
	jsonValue["HaveNewDevice"] = true;
	PublishToDeviceTelemetry(jsonValue);
}

Group *Gateway::getGroup(int id)
{
	if (groupList.find(id) != groupList.end())
	{
		return groupList[id];
	}
	return NULL;
}

Device *Gateway::getDevice(string mac)
{
	if (deviceList.find(mac) != deviceList.end())
	{
		return deviceList[mac];
	}
	return NULL;
}

#ifdef CONFIG_ENABLE_BLE
DeviceBle *Gateway::getDeviceBleFromAddr(uint32_t addr)
{
	for (const auto &[id, device] : deviceList)
	{
		if (device->CheckAddr(addr) && device->GetProtocol() >= BLE_DEVICE)
		{
			DeviceBle *deviceBle = dynamic_cast<DeviceBle *>(device);
			if (deviceBle)
				return deviceBle;
		}
	}
	return NULL;
}
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
DeviceZigbee *Gateway::getDeviceZigbeeFromAddr(uint32_t addr)
{
	for (const auto &[id, device] : deviceList)
	{
		if (device->CheckAddr(addr) && device->GetProtocol() >= ZIGBEE_DEVICE)
		{
			DeviceZigbee *deviceZigbee = dynamic_cast<DeviceZigbee *>(device);
			if (deviceZigbee)
				return deviceZigbee;
		}
	}
	return NULL;
}
#endif

#ifdef CONFIG_ENABLE_LORA
DeviceLora *Gateway::getDeviceLoraFromAddr(uint32_t addr)
{
	for (const auto &[id, device] : deviceList)
	{
		if (device->CheckAddr(addr) && device->GetProtocol() >= LORA_DEVICE)
		{
			DeviceLora *deviceLora = dynamic_cast<DeviceLora *>(device);
			if (deviceLora)
				return deviceLora;
		}
	}
	return NULL;
}
#endif

Device *Gateway::AddNewDevice(string id, string name, string mac, uint32_t addr, uint32_t type, bool addGateway, bool addDatabase)
{
	LOGI("Add new device id: %s, name: %s, mac: %s, addr: 0x%04X, type: 0x%04X", id.c_str(), name.c_str(), mac.c_str(), addr, type);
	Device *device = NULL;
#ifdef CONFIG_ENABLE_BLE
	if (type == BLE_DOWNLIGHT_SMT)
	{
		device = new DeviceBleDownLightSmt(id, name, mac, addr);
	}
	else if (type == BLE_SWITCH_4)
	{
		device = new DeviceBleSwitch4(id, name, mac, addr);
	}
	else if (type == BLE_DC_SCENE_CONTACT)
	{
		device = new DeviceBleDCSceneContact(id, name, mac, addr);
	}
	else if (type == BLE_TEMP_HUM_SENSOR)
	{
		device = new DeviceBleTempHumSensor(id, name, mac, addr);
	}
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
	if (type == ZIGBEE_LUMI_PLUG)
	{
		device = new DeviceZigbeeOnoff(id, name, mac, addr);
	}
	else if (type == ZIGBEE_TELINK_TLSR82xx)
	{
		device = new DeviceZigbeeTelinkOnoff(id, name, mac, addr);
	}
#endif
#ifdef CONFIG_ENABLE_LORA
	if (type == LORA_SENSOR_LIGHT)
	{
		device = new DeviceLoraLightSensor(id, name, mac, addr);
	}
	else if (type == LORA_SENSOR_SOIL_MOISTURE)
	{
		device = new DeviceLoraSoilMoisSensor(id, name, mac, addr);
	}
	else if (type == LORA_SENSOR_HUM_TEMP)
	{
		device = new DeviceLoraTempHumSensor(id, name, mac, addr);
	}
	else if (type == LORA_SENSOR_CO2)
	{
		device = new DeviceLoraCo2Sensor(id, name, mac, addr);
	}
	else if (type == LORA_SENSOR_EC)
	{
		device = new DeviceLoraECSensor(id, name, mac, addr);
	}
#endif
#ifdef CONFIG_ENABLE_MODBUS
	if (type == MODBUS_ES_SM_TH_01)
	{
		device = new DeviceModbusES_SM_TH_01(id, name, mac, addr);
	}
#endif

	if (device)
	{
		if (addGateway)
			deviceList[mac] = device;
		if (addDatabase)
			database->DeviceAdd(device);
		if (connected)
			device->PushAttributes();
	}
	else
	{
		LOGW("Add new device not support type: 0x%04X", type);
	}
	return device;
}

Group *Gateway::AddNewGroup(Group *group, bool addGateway, bool addDatabase)
{
	if (group)
	{
		if (addDatabase)
		{
			int rs = database->GroupAdd(group);
			if (rs)
			{
				LOGW("rs: %d", rs);
				return NULL;
			}
		}
		if (addGateway)
			groupList[group->GetId()] = group;
	}
	return group;
}

Scene *Gateway::AddScene(Json::Value &sceneValue, bool addGateway, bool addDatabase)
{
	// TODO: Check Scene id exist
	if (sceneValue.isMember("id") && sceneValue["id"].isInt() &&
			sceneValue.isMember("repeat") && sceneValue["repeat"].isInt() &&
			sceneValue.isMember("fullDay") && sceneValue["fullDay"].isBool() &&
			sceneValue.isMember("type") && sceneValue["type"].isString() &&
			sceneValue.isMember("input") && sceneValue["input"].isObject() &&
			sceneValue.isMember("output") && sceneValue["output"].isObject())
	{
		int id = sceneValue["id"].asInt();
		int repeat = sceneValue["repeat"].asInt();
		bool fullDay = sceneValue["fullDay"].asBool();
		string type = sceneValue["type"].asString();
		Scene *scene = NULL;
		if (!fullDay && sceneValue.isMember("startTime") && sceneValue["startTime"].isString() &&
				sceneValue.isMember("endTime") && sceneValue["endTime"].isString())
		{
			string startTime = sceneValue["startTime"].asString();
			string endTime = sceneValue["endTime"].asString();
			scene = new Scene(id, type, repeat, Util::ConvertStrTimeToInt(startTime), Util::ConvertStrTimeToInt(endTime));
		}
		else
		{
			scene = new Scene(id, type, repeat);
		}
		if (!scene)
		{
			LOGE("New scene error");
			return NULL;
		}
		Json::Value inputValue = sceneValue["input"];
		if (inputValue.isMember("timer") && inputValue["timer"].isObject())
		{
			Json::Value timerValue = inputValue["timer"];
			if (timerValue.isMember("repeat") && timerValue["repeat"].isInt() &&
					timerValue.isMember("time") && timerValue["time"].isString())
			{
				int repeat = timerValue["repeat"].asInt();
				string timerStr = timerValue["time"].asString();
				LOGI("Have Timer: %s", timerStr.c_str());
				int timer = Util::ConvertStrTimeToInt(timerStr);
				if (timer > 0)
				{
					SceneInputTimer *sceneInputTimer = new SceneInputTimer(scene, timer, repeat);
					scene->AddSceneInput(sceneInputTimer);
				}
			}
		}
		if (inputValue.isMember("device") && inputValue["device"].isArray())
		{
			Json::Value deviceSceneInputList = inputValue["device"];
			for (Json::Value::ArrayIndex i = 0; i < deviceSceneInputList.size(); i++)
			{
				Json::Value deviceSceneInputValue = deviceSceneInputList[i];
				if (deviceSceneInputValue.isObject())
				{
					if (deviceSceneInputValue.isMember("mac") && deviceSceneInputValue["mac"].isString() &&
							deviceSceneInputValue.isMember("data") && deviceSceneInputValue["data"].isObject())
					{
						string mac = deviceSceneInputValue["mac"].asString();
						Json::Value dataValue = deviceSceneInputValue["data"];
						Device *device = gateway->getDevice(mac);
						if (device)
						{
							SceneInputDevice *sceneInputDevice = new SceneInputDevice(scene, device, dataValue);
							scene->AddSceneInput(sceneInputDevice);
						}
					}
				}
			}
		}

		Json::Value outputValue = sceneValue["output"];
		if (outputValue.isMember("device") && outputValue["device"].isArray())
		{
			Json::Value deviceSceneOutputList = outputValue["device"];
			for (Json::Value::ArrayIndex i = 0; i < deviceSceneOutputList.size(); i++)
			{
				Json::Value deviceSceneOutputValue = deviceSceneOutputList[i];
				if (deviceSceneOutputValue.isObject())
				{
					if (deviceSceneOutputValue.isMember("mac") && deviceSceneOutputValue["mac"].isString() &&
							deviceSceneOutputValue.isMember("data") && deviceSceneOutputValue["data"].isObject())
					{
						Json::Value dataValue = deviceSceneOutputValue["data"];
						string mac = deviceSceneOutputValue["mac"].asString();
						Device *device = gateway->getDevice(mac);
						if (device)
						{
							SceneOutputDevice *sceneOutputDevice = new SceneOutputDevice(device, dataValue);
							scene->AddSceneOutput(sceneOutputDevice);
						}
					}
				}
			}
		}
		if (outputValue.isMember("group") && outputValue["group"].isArray())
		{
			Json::Value groupSceneOutputList = outputValue["group"];
			for (Json::Value::ArrayIndex i = 0; i < groupSceneOutputList.size(); i++)
			{
				Json::Value groupSceneOutputValue = groupSceneOutputList[i];
				if (groupSceneOutputValue.isObject())
				{
					if (groupSceneOutputValue.isMember("id") && groupSceneOutputValue["id"].isInt() &&
							groupSceneOutputValue.isMember("data") && groupSceneOutputValue["data"].isObject())
					{
						int id = groupSceneOutputValue["id"].asInt();
						Json::Value dataValue = groupSceneOutputValue["data"];
						Group *group = gateway->getGroup(id);
						if (group)
						{
							SceneOutputGroup *sceneOutputGroup = new SceneOutputGroup(group, dataValue);
							scene->AddSceneOutput(sceneOutputGroup);
						}
					}
				}
			}
		}
		if (outputValue.isMember("relay") && outputValue["relay"].isArray())
		{
			Json::Value relaySceneOutputList = outputValue["relay"];
			for (Json::Value::ArrayIndex i = 0; i < relaySceneOutputList.size(); i++)
			{
				Json::Value relaySceneOutputValue = relaySceneOutputList[i];
				if (relaySceneOutputValue.isObject())
				{
					if (relaySceneOutputValue.isMember("relay") && relaySceneOutputValue["relay"].isInt() &&
							relaySceneOutputValue.isMember("value") && relaySceneOutputValue["value"].isInt())
					{
						int relay = relaySceneOutputValue["relay"].asInt();
						int value = relaySceneOutputValue["value"].asInt();
						SceneOutputRelay *sceneOutputRelay = new SceneOutputRelay(relay, value);
						scene->AddSceneOutput(sceneOutputRelay);
					}
				}
			}
		}
		LOGI("Add Scene %d", scene->GetId());
		if (addGateway)
			sceneList[scene->GetId()] = scene;
		if (addDatabase)
		{
			string sceneStr = sceneValue.toString();
			sceneStr.erase(remove_if(sceneStr.begin(), sceneStr.end(), ::isspace), sceneStr.end());
			database->SceneAdd(scene->GetId(), sceneStr);
		}
		scene->Check();
		return scene;
	}
	else
	{
		LOGW("Scene format error");
	}
	return NULL;
}

void Gateway::OnTimerTest()
{
	LOGW("OnTimerTest");
}

void Gateway::PushRelayState(uint8_t relay)
{
	Json::Value values;
	values["rl0"] = (relay >> 0) & 0x01;
	values["rl1"] = (relay >> 1) & 0x01;
	values["rl2"] = (relay >> 2) & 0x01;
	values["rl3"] = (relay >> 3) & 0x01;
	values["rl4"] = (relay >> 4) & 0x01;
	values["rl5"] = (relay >> 5) & 0x01;
	values["rl6"] = (relay >> 6) & 0x01;
	values["rl7"] = (relay >> 7) & 0x01;
	PublishToDeviceTelemetry(values);
}

#endif
