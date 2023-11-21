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

	RegisterDeviceModel(BLE_DOWNLIGHT_SMT, "", "Downlight SMT");
	RegisterDeviceModel(BLE_DOWNLIGHT_COB_GOC_RONG, "", "Downlight COB");
	RegisterDeviceModel(BLE_DOWNLIGHT_COB_GOC_HEP, "", "Downlight trang trí");
	RegisterDeviceModel(BLE_DOWNLIGHT_COB_TRANG_TRI, "", "Downlight trang trí");
	RegisterDeviceModel(BLE_PANEL_TRON, "", "Panel tròn");
	RegisterDeviceModel(BLE_PANEL_VUONG, "", "Panel");
	RegisterDeviceModel(BLE_LED_OP_TRAN, "", "Ốp trần");
	RegisterDeviceModel(BLE_LED_OP_TUONG, "", "Đèn gắn tường");
	RegisterDeviceModel(BLE_LED_CHIEU_TRANH, "", "Đèn gắn tường");
	RegisterDeviceModel(BLE_TRACKLIGHT, "", "Tracklight");
	RegisterDeviceModel(BLE_LED_THA_TRAN, "", "Đèn thả trần");
	RegisterDeviceModel(BLE_LED_CHIEU_GUONG, "", "Đèn chiếu gương");
	RegisterDeviceModel(BLE_LED_DAY_LINEAR, "", "LED dây sáng trắng");
	RegisterDeviceModel(BLE_LED_TUBE_M16, "", "Đèn tube");
	RegisterDeviceModel(BLE_DEN_BAN, "", "Đèn bàn");
	RegisterDeviceModel(BLE_LED_FLOOD, "", "Đèn trang trí");
	RegisterDeviceModel(BLE_LED_RLT03_06W, "", "Đèn ray LED thanh x/g đổi màu RLT03.BLE.CW 130/6W 48V");
	RegisterDeviceModel(BLE_LED_RLT02_10W, "", "Đèn ray LED thanh đổi màu RLT02.BLE.CW 370/10W 48V");
	RegisterDeviceModel(BLE_LED_RLT02_20W, "", "Đèn ray LED thanh đổi màu RTL02.BLE.CW 670/20W 48V");
	RegisterDeviceModel(BLE_LED_RLT01_10W, "", "Đèn ray LED thanh đổi màu RLT01.BLE.CW 330/10W 48V");
	RegisterDeviceModel(BLE_LED_TRL08_20W, "", "Đèn LED Tracklight đôi đổi màu TRL08.BLE.CW 20W 48V");
	RegisterDeviceModel(BLE_LED_TRL08_10W, "", "Đèn LED Tracklight đổi màu TRL08.BLE.CW 10W 48V");
	RegisterDeviceModel(BLE_LED_RLT03_12W, "", "Đèn ray LED thanh x/g đổi màu RLT03.BLE.CW 240/12W 48V");
	RegisterDeviceModel(BLE_LED_DAY_RGB, "", "LED dây màu RGB");
	RegisterDeviceModel(BLE_LED_DAY_RGBCW, "", "LED dây màu");
	RegisterDeviceModel(BLE_LED_BULB, "", "LED bulb màu");
	RegisterDeviceModel(BLE_DOWNLIGHT_RGBCW, "", "Downlight màu");
	RegisterDeviceModel(BLE_LED_OP_TRAN_LOA, "", "Ốp trần có loa");
	RegisterDeviceModel(BLE_SWITCH_ONOFF, "", "Công tắc đèn");
	RegisterDeviceModel(BLE_SWITCH_1, "", "Công tắc 1 nút");
	RegisterDeviceModel(BLE_SWITCH_2, "", "Công tắc 2 nút");
	RegisterDeviceModel(BLE_SWITCH_3, "", "Công tắc 3 nút");
	RegisterDeviceModel(BLE_SWITCH_4, "", "Công tắc 4 nút");
	RegisterDeviceModel(BLE_SWITCH_WATER_HEATER, "", "Công tắc BNL");
	RegisterDeviceModel(BLE_SWITCH_CURTAIN, "", "Công tắc rèm");
	RegisterDeviceModel(BLE_SWITCH_RGB_1, "", "Công tắc 1 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_2, "", "Công tắc 2 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_3, "", "Công tắc 3 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_4, "", "Công tắc 4 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_WATER_HEATER, "", "Công tắc BNL");
	RegisterDeviceModel(BLE_SWITCH_RGB_CURTAIN, "", "Công tắc rèm");
	RegisterDeviceModel(BLE_SWITCH_ROOLING_DOOR, "", "Công tắc cửa cuốn");
	RegisterDeviceModel(BLE_SWITCH_RGB_1_SQUARE, "", "Công tắc 1 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_2_SQUARE, "", "Công tắc 2 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_3_SQUARE, "", "Công tắc 3 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_4_SQUARE, "", "Công tắc 4 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_CURTAIN_SQUARE, "", "Công tắc rèm");
	RegisterDeviceModel(BLE_DC_SCENE_CONTACT, "", "DKTX M2");
	RegisterDeviceModel(BLE_AC_SCENE_CONTACT, "", "DKTX âm tường");
	RegisterDeviceModel(BLE_AC_SCENE_SCREEN_TOUCH, "", "Màn hình DKTX");
	RegisterDeviceModel(BLE_REMOTE_M3, "", "DKTX M3");
	RegisterDeviceModel(BLE_REMOTE_M3_V2, "", "DKTX M3 V2");
	RegisterDeviceModel(BLE_REMOTE_M4, "", "DKTX M4");
	RegisterDeviceModel(BLE_AC_SCENE_CONTACT_RGB, "", "DKTX âm tường");
	RegisterDeviceModel(BLE_AC_SCENE_CONTACT_RGB_SQUARE, "", "DKTX âm tường vuông");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_1, "", "Công tắc cơ 1 nút");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_2, "", "Công tắc cơ 2 nút");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_3, "", "Công tắc cơ 3 nút");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_4, "", "Công tắc cơ 4 nút");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_WATER_HEATER, "", "Công tắc cơ BNL");

	RegisterDeviceModel(26001, "", "Ổ cắm đơn");
	RegisterDeviceModel(26002, "", "Ổ cắm kéo dài");
	RegisterDeviceModel(BLE_SOCKET_SWITCH, "", "Ổ cắm công tắc chữ nhật");
	RegisterDeviceModel(31001, "", "Cảm biến ánh sáng");

	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_DC, "", "Cảm biến chuyển động");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_AC, "", "Cảm biến chuyển động AC");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_AC_AMTRAN, "", "Cảm biến chuyển động âm trần");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_CB10, "", "Cảm biến chuyển động CB10");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_CB09, "", "Cảm biến chuyển động CB09");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_CB015_RADA, "", "Cảm biến chuyển động CB015 rada");
	RegisterDeviceModel(BLE_SMOKE_SENSOR, "", "Cảm biến khói");
	RegisterDeviceModel(BLE_DOOR_SENSOR, "", "Cảm biến cửa");
	RegisterDeviceModel(BLE_DOOR_SENSOR_CB16, "", "Cảm biến cửa");
	RegisterDeviceModel(BLE_PM_SENSOR, "", "Cảm biến bụi mịn");
	RegisterDeviceModel(BLE_TEMP_HUM_SENSOR, "", "Cảm biến nhiệt/ẩm");

	RegisterDeviceModel(41001, "", "Điều khiển hồng ngoại");
	RegisterDeviceModel(41101, "", "Điều hoà");
	RegisterDeviceModel(41201, "", "Quạt");
	RegisterDeviceModel(41301, "", "TIVI");
	RegisterDeviceModel(42001, "", "Ổ cắm đơn Wifi");
	RegisterDeviceModel(42002, "", "Ổ cắm thông minh Rạng Đông 4 cổng");
	RegisterDeviceModel(42003, "", "Ổ cắm thông minh Rạng Đông 6 cổng");
	RegisterDeviceModel(43001, "", "Công tắc Wifi 1 nút");
	RegisterDeviceModel(43002, "", "Công tắc Wifi 2 nút");
	RegisterDeviceModel(43003, "", "Công tắc Wifi 3 nút");
	RegisterDeviceModel(43004, "", "Công tắc Wifi 4 nút");
	RegisterDeviceModel(61001, "", "Camera Wifi");
	RegisterDeviceModel(61002, "", "Camera Dahua");
	RegisterDeviceModel(61003, "", "Camera HikVision");
	RegisterDeviceModel(71001, "", "Khoá cửa Wifi");
	RegisterDeviceModel(81101, "", "Zone");
	RegisterDeviceModel(81102, "", "Face");

	RegisterDeviceModel(BLE_REPEATER, "", "Bộ lặp sóng");

	RegisterDeviceModel(50331649, "", "AiHub");

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

uint32_t Device::ConverPidToDeviveType(uint16_t pid)
{
	uint8_t type1 = (pid >> 12) & 0x0F;
	uint8_t type2 = (pid >> 8) & 0x0F;
	uint8_t type3 = (pid) & 0xFF;
	return (type3+ (type2 * 1000) + (type1 * 10000));
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
