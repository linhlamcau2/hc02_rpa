#include "Device.h"
#include "Gateway.h"
#include "Log.h"
#include <functional>
#include <unistd.h>
#include <Util.h>

Device::Device(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version) : Object(id, addr, name)
{
	this->mac = mac;
	this->type = type;
	this->dataJson = dataJson;
	this->version = version;
	powerSource = POWER_UNKNOWN;

	lastOnlineState = false;
	lastTimeActive = 0;
	lastTimeCheckActive = 0;
}

Device::~Device()
{
}

string Device::GetMac()
{
	return mac;
}

Json::Value Device::GetData()
{
	return dataJson;
}

string Device::GetDeviceKey()
{
	return "";
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

int Device::BuildAttributesValue(Json::Value &pushDataValue)
{
	Json::Value deviceData;
	deviceData["gateway"] = "Farm Gateway RAL";
	deviceData["name"] = name;
	deviceData["mac"] = mac;
	deviceData["type"] = (int)type;
	pushDataValue[id] = deviceData;
	return CODE_OK;
}

void Device::DeviceInputData(uint8_t *data, int len, uint16_t addr)
{
	InputData(data, len, addr);
}

void Device::UpdateLastTimeActive()
{
	lastTimeActive = time(NULL);
}

void Device::CheckTrigger()
{
	LOGV("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

int Device::DoJsonArray(Json::Value &dataValue)
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
	return CODE_OK;
}

int Device::PushTelemetry()
{
	Json::Value pushData;
	int build = BuildTelemetryValue(pushData);
	if (build == 0)
	{
		return gateway->CloudPublish(pushData);
	}
	return CODE_ERROR;
}

int Device::PushTelemetry(Json::Value &jsonValue)
{
	if (!jsonValue.isNull())
	{
		Json::Value deviceData;
		Json::Value devicesData;
		Json::Value dataValue;
		deviceData["id"] = id;
		deviceData["data"] = jsonValue;
		devicesData.append(deviceData);
		dataValue["device"] = devicesData;
		gateway->pushDeviceUpdateLocal(dataValue);
		gateway->pushDeviceUpdateCloud(dataValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Device::PushAttributes()
{
	LOGD("PushAttributes");
	Json::Value pushData;
	int build = BuildAttributesValue(pushData);
	if (build == 0)
	{
		return gateway->CloudPublish(pushData);
	}
	return CODE_ERROR;
}

int Device::PushAttributes(Json::Value &jsonValue)
{
	if (!jsonValue.isNull())
		return gateway->CloudPublish(jsonValue);
	return CODE_ERROR;
}

// TODO: remove
static map<uint32_t, string> typeToNameList;
static map<string, uint32_t> modelToTypeList;
static map<uint32_t, uint32_t> bleTypeToGroupIdList;

void Device::InitDeviceModelList()
{
	bleTypeToGroupIdList[BLE_DOWNLIGHT_SMT] = BLE_DOWNLIGHT_SMT_GROUP;
	bleTypeToGroupIdList[BLE_DOWNLIGHT_COB_GOC_RONG] = BLE_DOWNLIGHT_COB_GOC_RONG_GROUP;
	bleTypeToGroupIdList[BLE_DOWNLIGHT_COB_GOC_HEP] = BLE_DOWNLIGHT_COB_GOC_HEP_GROUP;
	bleTypeToGroupIdList[BLE_DOWNLIGHT_COB_TRANG_TRI] = BLE_DOWNLIGHT_COB_TRANG_TRI_GROUP;
	bleTypeToGroupIdList[BLE_PANEL_TRON] = BLE_PANEL_TRON_GROUP;
	bleTypeToGroupIdList[BLE_PANEL_VUONG] = BLE_PANEL_VUONG_GROUP;
	bleTypeToGroupIdList[BLE_LED_OP_TRAN] = BLE_LED_OP_TRAN_GROUP;
	bleTypeToGroupIdList[BLE_LED_OP_TUONG] = BLE_LED_OP_TUONG_GROUP;
	bleTypeToGroupIdList[BLE_LED_CHIEU_TRANH] = BLE_LED_CHIEU_TRANH_GROUP;
	bleTypeToGroupIdList[BLE_TRACKLIGHT] = BLE_TRACKLIGHT_GROUP;
	bleTypeToGroupIdList[BLE_LED_THA_TRAN] = BLE_LED_THA_TRAN_GROUP;
	bleTypeToGroupIdList[BLE_LED_CHIEU_GUONG] = BLE_LED_CHIEU_GUONG_GROUP;
	bleTypeToGroupIdList[BLE_LED_DAY_LINEAR] = BLE_LED_DAY_LINEAR_GROUP;
	bleTypeToGroupIdList[BLE_LED_TUBE_M16] = BLE_LED_TUBE_M16_GROUP;
	bleTypeToGroupIdList[BLE_DEN_BAN] = BLE_DEN_BAN_GROUP;
	bleTypeToGroupIdList[BLE_LED_FLOOD] = BLE_LED_FLOOD_GROUP;
	bleTypeToGroupIdList[BLE_LED_DAY_RGB] = BLE_LED_DAY_RGB_GROUP;
	bleTypeToGroupIdList[BLE_LED_DAY_RGBCW] = BLE_LED_DAY_RGBCW_GROUP;
	bleTypeToGroupIdList[BLE_LED_BULB] = BLE_LED_BULB_GROUP;
	bleTypeToGroupIdList[BLE_DOWNLIGHT_RGBCW] = BLE_DOWNLIGHT_RGBCW_GROUP;
	bleTypeToGroupIdList[BLE_LED_OP_TRAN_LOA] = BLE_LED_OP_TRAN_LOA_GROUP;
	bleTypeToGroupIdList[BLE_LED_RLT03_06W] = BLE_LED_RLT03_06W_GROUP;
	bleTypeToGroupIdList[BLE_LED_RLT02_10W] = BLE_LED_RLT02_10W_GROUP;
	bleTypeToGroupIdList[BLE_LED_RLT02_20W] = BLE_LED_RLT02_20W_GROUP;
	bleTypeToGroupIdList[BLE_LED_RLT01_10W] = BLE_LED_RLT01_10W_GROUP;
	bleTypeToGroupIdList[BLE_LED_TRL08_20W] = BLE_LED_TRL08_20W_GROUP;
	bleTypeToGroupIdList[BLE_LED_TRL08_10W] = BLE_LED_TRL08_10W_GROUP;
	bleTypeToGroupIdList[BLE_LED_RLT03_12W] = BLE_LED_RLT03_12W_GROUP;

	RegisterDeviceModel(ZIGBEE_LUMI_PLUG, "lumi.plug", "Ổ cắm đơn Zigbee");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_SWITCH, "lumi.sensor_switch", "Chuông cửa Zigbee");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_TEMP_HUM, "lumi.sensor_ht", "Cam biet nhiet do do am");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_WLEAK_AQ1, "lumi.sensor_wleak.aq1", "Cam bien ro nuoc");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_MAGNET, "lumi.sensor_magnet", "Cam bien cua");
	RegisterDeviceModel(ZIGBEE_TUYA_SENSOR_MAGNET_TY0203, "TY0203", "Cam bien cua Tuya");
	RegisterDeviceModel(ZIGBEE_TUYA_SENSOR_PIR_RH3040, "RH3040", "Cảm biến chuyển động Zigbee");
	RegisterDeviceModel(ZIGBEE_TUYA_SENSOR_HUMAN_PRESENCE_TS0225, "TS0225", "Cảm biến nhan dien nguoi Zigbee");
}

void Device::RegisterDeviceModel(uint32_t type, string model, string name)
{
	typeToNameList[type] = name;
	if (model != "")
		modelToTypeList[model] = type;
}

uint32_t Device::BleTypeToGroupId(uint32_t deviceType)
{
	return bleTypeToGroupIdList[deviceType];
}

uint32_t Device::ConvertModelToDeviceType(string model)
{
	return modelToTypeList[model];
}

string Device::ConvertDeviceTypeToName(uint32_t type)
{
	return typeToNameList[type];
}

bool Device::GetIsFavorite()
{
	return this->isFavorite;
}

bool Device::SetIsFavorite(bool isFavorite)
{
	this->isFavorite = isFavorite;
	return this->isFavorite;
}