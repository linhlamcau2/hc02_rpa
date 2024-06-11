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
	this->isFavorite = false;
	propertyJsonUpdate = Json::objectValue;

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

void Device::SetPropertyJsonUpdate(Json::Value property)
{
	if (this->propertyJsonUpdate != property)
	{
		PushTelemetry(property);
		this->propertyJsonUpdate = property;
	}
}

void Device::UpdatePropertyJsonUpdate(Json::Value &propertyUpdate)
{
	if (propertyUpdate.isObject() && this->propertyJsonUpdate.isObject())
	{
		for (auto const &key : propertyUpdate.getMemberNames())
		{
			if (this->propertyJsonUpdate.isMember(key))
			{
				if (propertyUpdate[key].type() == this->propertyJsonUpdate[key].type())
				{
					this->propertyJsonUpdate[key] = propertyUpdate[key];
				}
			}
		}
	}
}

Json::Value Device::GetPropertyJsonUpdate()
{
	return this->propertyJsonUpdate;
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
static map<uint32_t, const char *> typeToNameList;
static map<uint32_t, const char *> typeToModelList;
// static map<string, uint32_t> modelToTypeList;
static map<uint32_t, uint32_t> bleTypeToGroupIdList;
static map<uint16_t, const char *> bleAttributeIdToAttributeString;

// TODO: Check list device to Name
void Device::InitDeviceModelList()
{
	bleTypeToGroupIdList[TYPE_GROUP_SWITCH] = BLE_SWITCH_TOUCH_GROUP;

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
	bleTypeToGroupIdList[BLE_SWITCH_ONOFF_V2] = BLE_SWITCH_ONOFF_V2_GROUP;

	// bleTypeToGroupIdList[BLE_SWITCH_RGB_1] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_3] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_4] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_1_SQUARE] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_2_SQUARE] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_3_SQUARE] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_4_SQUARE] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_1_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_1_SQUARE_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_2_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_2_SQUARE_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_3_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_3_SQUARE_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_4_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_1] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_3] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_4] = BLE_SWITCH_TOUCH_GROUP;

	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_1] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_2] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_3] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_1_V2] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_2_V2] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_3_V2] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_ELECTRICAL_1] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_ELECTRICAL_2] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_ELECTRICAL_3] = BLE_SWITCH_ELECTRICAL_GROUP;

	// bleTypeToGroupIdList[BLE_SWITCH_2_CEILING] = BLE_SWITCH_CEILING_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_3_CEILING] = BLE_SWITCH_CEILING_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_5_CEILING] = BLE_SWITCH_CEILING_GROUP;

	bleTypeToGroupIdList[BLE_SWITCH_RGB_CURTAIN_HCN] = BLE_SWITCH_CURTAIN_GROUP;
	bleTypeToGroupIdList[BLE_SWITCH_RGB_CURTAIN_SQUARE_V2] = BLE_SWITCH_CURTAIN_GROUP;

	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_ONOFF] = KEY_ATTRIBUTE_ONOFF;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DIM] = KEY_ATTRIBUTE_DIM;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CCT] = KEY_ATTRIBUTE_CCT;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_HUE] = KEY_ATTRIBUTE_HUE;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SATURATION] = KEY_ATTRIBUTE_SATURATION;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_LUMINANCE] = KEY_ATTRIBUTE_LUMINANCE;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SONG] = KEY_ATTRIBUTE_SONG;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BLINK_MODE] = KEY_ATTRIBUTE_BLINK_MODE;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BATTERY] = KEY_ATTRIBUTE_BATTERY;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_LUX] = KEY_ATTRIBUTE_LUX;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_PIR] = KEY_ATTRIBUTE_PIR;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_1] = KEY_ATTRIBUTE_BUTTON;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_2] = KEY_ATTRIBUTE_BUTTON "2";
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_3] = KEY_ATTRIBUTE_BUTTON "3";
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_4] = KEY_ATTRIBUTE_BUTTON "4";
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_5] = KEY_ATTRIBUTE_BUTTON "5";
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_6] = KEY_ATTRIBUTE_BUTTON "6";
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_ACTIME] = KEY_ATTRIBUTE_ACTIME;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_PM2_5] = KEY_ATTRIBUTE_PM2_5;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_PM10] = KEY_ATTRIBUTE_PM10;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_PM1_0] = KEY_ATTRIBUTE_PM1_0;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_TEMP] = KEY_ATTRIBUTE_TEMP;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_HUMIDITY] = KEY_ATTRIBUTE_HUMIDITY;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SCENE_RGB] = KEY_ATTRIBUTE_MODE_RGB;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_HANGON] = KEY_ATTRIBUTE_HANGON;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_COUNTDOWN] = KEY_ATTRIBUTE_COUNTDOWN;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_AIR_CONDITIONER_WIND] = KEY_ATTRIBUTE_AIR_CONDITIONER_WIND;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_AIR_CONDITIONER_MODE] = KEY_ATTRIBUTE_AIR_CONDITIONER_MODE;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_AIR_CONDITIONER_TEMP] = KEY_ATTRIBUTE_AIR_CONDITIONER_TEMP;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CURTAIN_OPEN] = KEY_ATTRIBUTE_CURTAIN_OPEN;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CURTAIN_CLOSE] = KEY_ATTRIBUTE_CURTAIN_CLOSE;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CURTAIN_PAUSE] = KEY_ATTRIBUTE_CURTAIN_PAUSE;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CURTAIN_OPENED] = KEY_ATTRIBUTE_CURTAIN_OPENED;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SMOKE] = KEY_ATTRIBUTE_SMOKE;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DOOR] = KEY_ATTRIBUTE_DOOR;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SMOKE_PIN] = KEY_ATTRIBUTE_SMOKE_PIN;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DKTX_SCENE] = KEY_ATTRIBUTE_DKTX_SCENE;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_ONLINE_OFFLINE] = KEY_ATTRIBUTE_ONLINE_OFFLINE;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_MOTOR] = KEY_ATTRIBUTE_MOTOR;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_R] = KEY_ATTRIBUTE_R;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_G] = KEY_ATTRIBUTE_G;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_B] = KEY_ATTRIBUTE_B;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DIM_ON] = KEY_ATTRIBUTE_DIM_ON;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DIM_OFF] = KEY_ATTRIBUTE_DIM_OFF;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SENSI_SENSOR] = KEY_ATTRIBUTE_SENSI;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DISTANCE] = KEY_ATTRIBUTE_DISTANCE;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_INPUT_MODE] = KEY_ATTRIBUTE_MODE_INPUT;
	bleAttributeIdToAttributeString[BLE_ATTRIBUTE_STARTUP] = KEY_ATTRIBUTE_STATUS_STARTUP;

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
	RegisterDeviceModel(BLE_SWITCH_ONOFF_V2, "", "Công tắc đèn");
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

	RegisterDeviceModel(BLE_SOCKET, "", "Ổ cắm đơn");
	RegisterDeviceModel(BLE_SOCKET_EXTEN, "", "Ổ cắm kéo dài");
	RegisterDeviceModel(BLE_SWITCH_RGB_SOCKET_1, "", "Ổ cắm công tắc chữ nhật");
	RegisterDeviceModel(BLE_LIGHT_SENSOR, "", "Cảm biến ánh sáng");

	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_DC, "", "Cảm biến chuyển động");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_AC, "", "Cảm biến chuyển động AC");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_AC_AMTRAN, "", "Cảm biến chuyển động âm trần");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_DC_CB10, "", "Cảm biến chuyển động CB10");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_DC_CB09, "", "Cảm biến chuyển động CB09");
	RegisterDeviceModel(BLE_RADA_LIGHT_SENSOR_AC_CB15, "", "Cảm biến chuyển động CB015 rada");
	RegisterDeviceModel(BLE_SMOKE_SENSOR, "", "Cảm biến khói");
	RegisterDeviceModel(BLE_DOOR_SENSOR, "", "Cảm biến cửa");
	RegisterDeviceModel(BLE_DOOR_CB16_SENSOR, "", "Cảm biến cửa");
	RegisterDeviceModel(BLE_PM_SENSOR, "", "Cảm biến bụi mịn");
	RegisterDeviceModel(BLE_TEMP_HUM_SENSOR, "", "Cảm biến nhiệt/ẩm");

	RegisterDeviceModel(WIFI_IR, "", "Điều khiển hồng ngoại");
	RegisterDeviceModel(WIFI_IR_AIRCONDITION, "", "Điều hoà");
	RegisterDeviceModel(WIFI_IR_FAN, "", "Quạt");
	RegisterDeviceModel(WIFI_IR_TV, "", "TIVI");
	RegisterDeviceModel(WIFI_SOCKET, "", "Ổ cắm đơn Wifi");
	RegisterDeviceModel(WIFI_SOCKET_4, "", "Ổ cắm thông minh Rạng Đông 4 cổng");
	RegisterDeviceModel(WIFI_SOCKET_6, "", "Ổ cắm thông minh Rạng Đông 6 cổng");
	RegisterDeviceModel(WIFI_SWITCH_1, "", "Công tắc Wifi 1 nút");
	RegisterDeviceModel(WIFI_SWITCH_2, "", "Công tắc Wifi 2 nút");
	RegisterDeviceModel(WIFI_SWITCH_3, "", "Công tắc Wifi 3 nút");
	RegisterDeviceModel(WIFI_SWITCH_4, "", "Công tắc Wifi 4 nút");
	RegisterDeviceModel(CAMERA_TUYA, "", "Camera Wifi");
	RegisterDeviceModel(CAMERA_DAHUA, "", "Camera Dahua");
	RegisterDeviceModel(CAMERA_HKVISION, "", "Camera HikVision");
	RegisterDeviceModel(WIFI_DOOR_LOCK, "", "Khoá cửa Wifi");
	RegisterDeviceModel(AI_ZONE, "", "Zone");
	RegisterDeviceModel(AI_FACE, "", "Face");

	RegisterDeviceModel(BLE_REPEATER, "", "Bộ lặp sóng");

	RegisterDeviceModel(MQTT_AI_HUB, "", "AiHub");

	RegisterDeviceModel(ZIGBEE_LUMI_PLUG, "lumi.plug", "Ổ cắm đơn Zigbee");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_SWITCH, "lumi.sensor_switch", "Chuông cửa Zigbee");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_TEMP_HUM, "lumi.sensor_ht", "Cam biet nhiet do do am");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_WLEAK_AQ1, "lumi.sensor_wleak.aq1", "Cam bien ro nuoc");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_MAGNET, "lumi.sensor_magnet", "Cam bien cua");
	RegisterDeviceModel(ZIGBEE_TUYA_SENSOR_MAGNET_TY0203, "TY0203", "Cam bien cua Tuya");
	RegisterDeviceModel(ZIGBEE_TUYA_SENSOR_PIR_RH3040, "RH3040", "Cảm biến chuyển động Zigbee");
	RegisterDeviceModel(ZIGBEE_TUYA_SENSOR_HUMAN_PRESENCE_TS0225, "TS0225", "Cảm biến nhan dien nguoi Zigbee");
}

void Device::RegisterDeviceModel(uint32_t type, const char *model, const char *name)
{
	typeToNameList[type] = name;
	typeToModelList[type] = model;
	// if (model != "")
	// 	modelToTypeList[model] = type;
}

uint32_t Device::BleTypeToGroupId(uint32_t deviceType)
{
	return bleTypeToGroupIdList[deviceType];
}

const char *Device::BleAttributeIdToAttributeStr(uint16_t attributeId)
{
	return bleAttributeIdToAttributeString[attributeId];
}

// uint32_t Device::ConvertModelToDeviceType(string model)
// {
// 	return modelToTypeList[model];
// }

const char *Device::ConvertDeviceTypeToName(uint32_t type)
{
	return typeToNameList[type];
}

const char *Device::ConvertDeviceTypeToModel(uint32_t type)
{
	return typeToModelList[type];
}

uint32_t Device::ConverPidToDeviveType(uint16_t pid)
{
	uint8_t type1 = (pid >> 12) & 0x0F;
	uint8_t type2 = (pid >> 8) & 0x0F;
	uint8_t type3 = (pid) & 0xFF;
	return (type3 + (type2 * 1000) + (type1 * 10000));
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
