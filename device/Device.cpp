#include "Device.h"
#include "Gateway.h"
#include "Log.h"
#include <thread>
#include <functional>
#include <unistd.h>

Device::Device(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version) : Object(id, addr, name)
{
	this->mac = mac;
	this->type = type;
	this->device_id = device_id;
	this->version = version;
	countElement = 1;
	powerSource = POWER_UNKNOWN;

	lastOnlineState = false;
	lastTimeActive = 0;
	lastTimeCheck = 0;
}

Device::~Device()
{
}

string Device::GetMac()
{
	return mac;
}

string Device::GetDeviceId()
{
	return device_id;
}

bool Device::CheckAddr(uint32_t addr)
{
	return ((this->addr <= addr) && (this->addr + countElement - 1 >= addr));
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

bool Device::isOnline()
{
	return lastOnlineState;
}

bool Device::isNeedCheckOnline()
{
	return powerSource == POWER_AC;
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
	for (auto &module : modules)
	{
		module->BuildTelemetryValue(pushDataValue);
	}
	for (auto &element : elements)
	{
		element->BuildTelemetryValue(pushDataValue);
	}
	return CODE_OK;
}

int Device::BuildTelemetryValueV2(Json::Value &pushDataValue)
{
	for (auto &module : modules)
	{
		module->BuildTelemetryValueV2(pushDataValue);
	}
	for (auto &element : elements)
	{
		element->BuildTelemetryValueV2(pushDataValue);
	}
	return CODE_OK;
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

void Device::DeviceInputData(uint8_t *data, int len, uint32_t addr)
{
	lastTimeActive = time(NULL);
	InputData(data, len, addr);
}

void Device::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	for (auto &module : modules)
	{
		if (module->InputData(data, len, values) == CODE_OK)
			break;
	}
	for (auto &element : elements)
	{
		if (element->CheckAddr(addr))
		{
			if (element->InputData(data, len, values) == CODE_OK)
				break;
		}
	}
	PushTelemetry(values);
}

bool Device::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	for (auto &module : modules)
	{
		if (module->CheckData(dataValue, rs) == CODE_OK)
			return true;
	}
	for (auto &element : elements)
	{
		if (element->CheckData(dataValue, rs) == CODE_OK)
			return true;
	}
	return false;
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

bool Device::DoJsonArray(Json::Value &dataValue)
{
	if (dataValue.isArray())
	{
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			Do(dataValue[i]);
		}
	}
	else
	{
		Do(dataValue);
	}
	return true;
}

bool Device::DoJsonArrayV2(Json::Value &dataValue)
{
	if (dataValue.isArray())
	{
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			DoV2(dataValue[i]);
		}
	}
	else
	{
		DoV2(dataValue);
	}
	return true;
}

bool Device::Do(Json::Value &dataValue)
{
	for (auto &module : modules)
	{
		module->Do(dataValue);
	}
	for (auto &element : elements)
	{
		element->Do(dataValue);
	}
	return CODE_OK;
}

bool Device::DoV2(Json::Value &dataValue)
{
	for (auto &module : modules)
	{
		module->DoV2(dataValue);
	}
	for (auto &element : elements)
	{
		element->DoV2(dataValue);
	}
	return CODE_OK;
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

// TODO: remove
static map<uint32_t, string> typeToNameList;
static map<string, uint32_t> modelToTypeList;

void Device::InitDeviceModelList()
{
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
