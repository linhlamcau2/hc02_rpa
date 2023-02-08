#include "Device.h"
#include "Gateway.h"
#include <thread>
#include <functional>
#include <Log.h>
#include <unistd.h>

Device::Device(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version)
{
	this->id = id;
	this->name = name;
	this->mac = mac;
	this->addr = addr;
	this->type = type;
	this->device_id = device_id;
	this->version = version;
}

Device::~Device()
{
}

string Device::GetId()
{
	return id;
}

string Device::GetName()
{
	return name;
}

string Device::GetMac()
{
	return mac;
}

string Device::GetDeviceId()
{
	return device_id;
}

uint32_t Device::GetAddr()
{
	return addr;
}

bool Device::CheckAddr(uint32_t addr)
{
	return this->addr == addr;
}

void Device::SetAddr(uint32_t addr)
{
	this->addr = addr;
}

uint32_t Device::GetType()
{
	return type;
}

uint16_t Device::GetVersion()
{
	return version;
}

string Device::GetVersionStr()
{
	return to_string((version >> 8) & 0xFF) + "." + to_string(version & 0xFF);
}

int Device::GetRSSI()
{
	return rssi;
}

void Device::SetRSSI(int rssi)
{
	this->rssi = rssi;
}

protocol_e Device::GetProtocol()
{
	return protocol;
}

void Device::RegisterTrigger(RuleInputDevice *ruleInputDevice)
{
	LOGD("RegisterTrigger");
	deviceRuleInputList.push_back(ruleInputDevice);
}

void Device::UnregisterTrigger(RuleInputDevice *ruleInputDevice)
{
	LOGD("UnregisterTrigger");
	deviceRuleInputList.erase(remove(deviceRuleInputList.begin(), deviceRuleInputList.end(), ruleInputDevice), deviceRuleInputList.end());
}

int Device::BuildTelemetryValue(Json::Value &pushDataValue)
{
	LOGW("BuildTelemetryValue");
	return -1;
}

int Device::BuildAttributesValue(Json::Value &pushDataValue)
{
	Json::Value deviceData;
	deviceData["gateway"] = "Farm Gateway RAL";
	deviceData["name"] = name;
	deviceData["mac"] = mac;
	deviceData["type"] = (int)type;
	pushDataValue[id] = deviceData;
	return 0;
}

void Device::CheckTrigger()
{
	LOGD("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

int Device::PushTelemetry()
{
	Json::Value pushData;
	int build = BuildTelemetryValue(pushData);
	if (build == 0)
	{
		return gateway->PublishToGatewayTelemetry(pushData);
	}
	return -1;
}

int Device::PushTelemetry(Json::Value jsonValue)
{
	if (jsonValue.isNull())
		return -1;
	Json::Value pushDataValue;
	Json::Value deviceData;
	deviceData["DEVICE_ID"] = id;
	deviceData["PROPERTIES"] = jsonValue;
	pushDataValue["CMD"] = "DEVICE";
	pushDataValue["DATA"].append(deviceData);
	gateway->PublishToLocalMessage(pushDataValue);
	return gateway->PublishToGatewayTelemetry(pushDataValue);
}

int Device::PushAttributes()
{
	LOGD("PushAttributes");
	Json::Value pushData;
	int build = BuildAttributesValue(pushData);
	if (build == 0)
	{
		return gateway->PublishToGatewayAttributes(pushData);
	}
	return -1;
}

int Device::PushAttributes(Json::Value jsonValue)
{
	if (jsonValue.isNull())
		return -1;
	return gateway->PublishToGatewayAttributes(jsonValue);
}

map<string, int> parameterToId;
static map<uint32_t, string> typeToNameList;
static map<string, uint32_t> modelToTypeList;

void Device::InitDeviceModelList()
{
	parameterToId["stt"] = 0;
	parameterToId["dim"] = 1;
	parameterToId["pin"] = 8;
	parameterToId["bt0"] = 11;
	parameterToId["onoff0"] = 11;
	parameterToId["bt1"] = 12;
	parameterToId["onoff1"] = 12;
	parameterToId["bt2"] = 13;
	parameterToId["onoff2"] = 13;
	parameterToId["bt3"] = 14;
	parameterToId["onoff3"] = 14;
	parameterToId["bt4"] = 15;
	parameterToId["onoff4"] = 15;
	parameterToId["bt5"] = 16;
	parameterToId["onoff5"] = 16;
	parameterToId["temp"] = 21;
	parameterToId["hum"] = 22;
	parameterToId["modeRgb"] = 23;

	RegisterDeviceModel(ZIGBEE_LUMI_PLUG, "lumi.plug", "Ổ cắm đơn Zigbee");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_SWITCH, "lumi.sensor_switch", "Chuông cửa Zigbee");
	RegisterDeviceModel(ZIGBEE_PIR_RH3040, "RH3040", "Cảm biến chuyển động Zigbee");
	RegisterDeviceModel(ZIGBEE_TELINK_TLSR82xx, "TLSR82xx", "Đèn Telink Zigbee");
}

void Device::RegisterDeviceModel(uint32_t type, string model, string name)
{
	typeToNameList[type] = name;
	if (model != "")
		modelToTypeList[model] = type;
}

uint32_t Device::ConvertModelToDeviceType(string model)
{
	return modelToTypeList[model];
}

string Device::ConvertDeviceTypeToName(uint32_t type)
{
	return typeToNameList[type];
}
