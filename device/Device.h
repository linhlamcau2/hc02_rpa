#pragma once

#include <string>
#include <vector>
#include <byteswap.h>
#include "json.h"
#include "ErrorCode.h"
#include "Object.h"
#include "RuleInputDevice.h"

#define KEY_ATTRIBUTE_ONOFF "onoff"
#define KEY_ATTRIBUTE_DIM "dim"
#define KEY_ATTRIBUTE_CCT "cct"
#define KEY_ATTRIBUTE_HUE "h"
#define KEY_ATTRIBUTE_SATURATION "s"
#define KEY_ATTRIBUTE_LUMINANCE "l"
#define KEY_ATTRIBUTE_SONG "song"
#define KEY_ATTRIBUTE_BLINK_MODE "blm"
#define KEY_ATTRIBUTE_BATTERY "bat"
#define KEY_ATTRIBUTE_LUX "lux"
#define KEY_ATTRIBUTE_PIR "pir"
#define KEY_ATTRIBUTE_BUTTON "bt"
#define KEY_ATTRIBUTE_ACTIME "actime"
#define KEY_ATTRIBUTE_ACMODE "acmode"
#define KEY_ATTRIBUTE_SENSI "sensi"
#define KEY_ATTRIBUTE_PM2_5 "pm2.5"
#define KEY_ATTRIBUTE_PM10 "pm10"
#define KEY_ATTRIBUTE_PM1_0 "pm1.0"
#define KEY_ATTRIBUTE_TEMP "temp"
#define KEY_ATTRIBUTE_HUMIDITY "hum"
#define KEY_ATTRIBUTE_MODE_RGB "sceneRGB"
#define KEY_ATTRIBUTE_HANGON "hangon"
#define KEY_ATTRIBUTE_COUNTDOWN "countdown"
#define KEY_ATTRIBUTE_AIR_CONDITIONER_WIND "airConditionerWind"
#define KEY_ATTRIBUTE_AIR_CONDITIONER_MODE "airConditionerMode"
#define KEY_ATTRIBUTE_AIR_CONDITIONER_TEMP "airConditionerTemp"
#define KEY_ATTRIBUTE_CURTAIN_OPEN "curtainOpen"
#define KEY_ATTRIBUTE_CURTAIN_CLOSE "curtainClose"
#define KEY_ATTRIBUTE_CURTAIN_PAUSE "curtainPause"
#define KEY_ATTRIBUTE_CURTAIN_OPENED "curtainOpened"
#define KEY_ATTRIBUTE_SMOKE "smoke"
#define KEY_ATTRIBUTE_DOOR "door"
#define KEY_ATTRIBUTE_SMOKE_PIN "smokePin"
#define KEY_ATTRIBUTE_DKTX_SCENE "remoteScene"
#define KEY_ATTRIBUTE_ONLINE_OFFLINE "status"
#define KEY_ATTRIBUTE_MOTOR "motor"
#define KEY_ATTRIBUTE_R "r"
#define KEY_ATTRIBUTE_G "g"
#define KEY_ATTRIBUTE_B "b"
#define KEY_ATTRIBUTE_DIM_ON "dimOn"
#define KEY_ATTRIBUTE_DIM_OFF "dimOff"
#define KEY_ATTRIBUTE_RELAY "rl"
#define KEY_ATTRIBUTE_DISTANCE "distance"
#define KEY_ATTRIBUTE_STATUS_STARTUP "statusStartup"
#define KEY_ATTRIBUTE_MODE_INPUT "modeInput"

#define KEY_ATTRIBUTE_ZONE_ID "zoneId"
#define KEY_ATTRIBUTE_ZONE_VALUE "zoneValue"
#define KEY_ATTRIBUTE_FACE_ID "faceId"
#define KEY_ATTRIBUTE_FACE_VALUE "faceValue"

#define KEYJSON_GEN_DEVICEID "genIdChildNew"

using namespace std;

typedef enum
{
	BLE_DEVICE,
	ZIGBEE_DEVICE,
	MQTT_DEVICE
} protocol_e;

enum
{
	BLE_ALL = 0,
	BLE_DOWNLIGHT_SMT = 12001, // 0x010201,
	BLE_DOWNLIGHT_COB_GOC_RONG = 12002,
	BLE_DOWNLIGHT_COB_GOC_HEP = 12003,
	BLE_DOWNLIGHT_COB_TRANG_TRI = 12004,
	BLE_PANEL_TRON = 12005,
	BLE_PANEL_VUONG = 12006,
	BLE_LED_OP_TRAN = 12007,
	BLE_LED_OP_TUONG = 12008,
	BLE_LED_CHIEU_TRANH = 12009,
	BLE_TRACKLIGHT = 12010,
	BLE_LED_THA_TRAN = 12011,
	BLE_LED_CHIEU_GUONG = 12012,
	BLE_LED_DAY_LINEAR = 12013,
	BLE_LED_TUBE_M16 = 12014,
	BLE_DEN_BAN = 12015,
	BLE_LED_FLOOD = 12016,
	BLE_LED_RLT03_06W = 12020,
	BLE_LED_RLT02_10W = 12021,
	BLE_LED_RLT02_20W = 12022,
	BLE_LED_RLT01_10W = 12023,
	BLE_LED_TRL08_20W = 12024,
	BLE_LED_TRL08_10W = 12025,
	BLE_LED_RLT03_12W = 12026,
	BLE_LED_OP_TRAN_40W = 12027,
	BLE_LED_DAY_RGB = 13001,
	BLE_LED_DAY_RGBCW = 14001,
	BLE_LED_BULB = 14002,
	BLE_DOWNLIGHT_RGBCW = 14003,
	BLE_LED_OP_TRAN_LOA = 15001,

	BLE_SWITCH_ONOFF = 21001,

	BLE_SWITCH_1 = 22001,
	BLE_SWITCH_2 = 22002,
	BLE_SWITCH_3 = 22003,
	BLE_SWITCH_4 = 22004,
	BLE_SWITCH_WATER_HEATER = 22005,
	BLE_SWITCH_RGB_1 = 22012,
	BLE_SWITCH_RGB_2 = 22013,
	BLE_SWITCH_RGB_3 = 22014,
	BLE_SWITCH_RGB_4 = 22015,
	BLE_SWITCH_RGB_WATER_HEATER = 22016,
	BLE_SWITCH_RGB_1_SQUARE = 22019,
	BLE_SWITCH_RGB_2_SQUARE = 22020,
	BLE_SWITCH_RGB_3_SQUARE = 22021,
	BLE_SWITCH_RGB_4_SQUARE = 22022,
	BLE_SWITCH_RGB_1_V2 = 22026,
	BLE_SWITCH_RGB_1_SQUARE_V2 = 22027,
	BLE_SWITCH_RGB_2_V2 = 22028,
	BLE_SWITCH_RGB_2_SQUARE_V2 = 22029,
	BLE_SWITCH_RGB_3_V2 = 22030,
	BLE_SWITCH_RGB_3_SQUARE_V2 = 22031,
	BLE_SWITCH_RGB_4_V2 = 22032,
	BLE_SWITCH_RGB_4_SQUARE_V2 = 22033,
	BLE_SWITCH_2_CEILING = 22037,
	BLE_SWITCH_3_CEILING = 22038,
	BLE_SWITCH_5_CEILING = 22039,

	BLE_SWITCH_CURTAIN = 22006,
	BLE_SWITCH_RGB_CURTAIN = 22017,
	BLE_SWITCH_RGB_CURTAIN_SQUARE = 22024,
	BLE_SWITCH_RGB_CURTAIN_HCN = 22034,
	BLE_SWITCH_RGB_CURTAIN_SQUARE_V2 = 22035,

	BLE_SWITCH_ROOLING_DOOR = 22018,
	BLE_SWITCH_ROOLING_DOOR_V2 = 22025,
	BLE_SWITCH_ROOLING_DOOR_SQUARE = 22036,

	BLE_DC_SCENE_CONTACT = 23001, // 0x020301,
	BLE_AC_SCENE_CONTACT = 23002, // 0x020302,
	BLE_AC_SCENE_CONTACT_RGB = 23006,
	BLE_AC_SCENE_CONTACT_RGB_SQUARE = 23008,
	BLE_AC_SCENE_SCREEN_TOUCH = 23003,
	BLE_REMOTE_M3 = 23004,
	BLE_REMOTE_M3_V2 = 23007,
	BLE_REMOTE_M4 = 23005,

	BLE_SWITCH_ELECTRICAL_1 = 24001,
	BLE_SWITCH_ELECTRICAL_2 = 24002,
	BLE_SWITCH_ELECTRICAL_3 = 24003,
	BLE_SWITCH_ELECTRICAL_4 = 24004,
	BLE_SWITCH_ELECTRICAL_WATER_HEATER = 24005,

	BLE_SWITCH_RGB_SOCKET_1 = 26003,

	BLE_PM_SENSOR = 37001,
	BLE_TEMP_HUM_SENSOR = 38001, // 0x030801,
	BLE_PIR_LIGHT_SENSOR_DC = 32001,
	BLE_PIR_LIGHT_SENSOR_AC = 32002,
	BLE_PIR_LIGHT_SENSOR_DC_CB09 = 32007,
	BLE_PIR_LIGHT_SENSOR_DC_CB10 = 32006,
	BLE_PIR_LIGHT_SENSOR_AC_AMTRAN = 32004,
	BLE_RADA_LIGHT_SENSOR_AC_CB15 = 32008,
	BLE_DOOR_SENSOR = 36001,
	BLE_DOOR_CB16_SENSOR = 36002,
	BLE_SMOKE_SENSOR = 33001,

	BLE_REPEATER = 91001,

	ZIGBEE_LUMI_PLUG = 0x02000001,
	ZIGBEE_LUMI_SENSOR_SWITCH = 0x02000002,
	ZIGBEE_PIR_RH3040 = 0x02000102,
	ZIGBEE_TELINK_TLSR82xx = 0x02000201,

	MQTT_AI_HUB = 0x03000001
};

enum
{
	POWER_UNKNOWN = 0,
	POWER_BATTERY,
	POWER_AC,
};

class Device : public Object
{
protected:
	string mac;
	uint32_t type;
	uint16_t version;
	int rssi;
	protocol_e protocol;
	string data;
	bool isFavorite;
	Json::Value values;

public:
	vector<RuleInputDevice *> deviceRuleInputList;

	int powerSource;
	bool lastOnlineState;
	time_t lastTimeActive;
	time_t lastTimeCheckActive;

public:
	Device(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
	virtual ~Device();

	string GetMac();
	uint32_t GetType();
	uint16_t GetVersion();
	string GetVersionStr();
	string GetData();
	int GetRSSI();

	void SetRSSI(int rssi);
	virtual bool CheckAddr(uint32_t addr) { return this->addr == addr; }
	virtual bool CheckId(string id) { return this->id == id; }
	virtual string GetDeviceKey();

	protocol_e GetProtocol();

	bool isOnline();
	bool isNeedCheckOnline();
	void UpdateLastTimeActive();

	bool GetIsFavorite();
	bool SetIsFavorite(bool isFavorite);

	void RegisterTrigger(RuleInputDevice *ruleInputDevice);
	void UnregisterTrigger(RuleInputDevice *ruleInputDevice);

	virtual int BuildAttributesValue(Json::Value &pushDataValue);

	void DeviceInputData(uint8_t *data, int len, uint32_t addr);

	virtual void InitAttribute(int attributeId, double value) {}
	virtual void CheckTrigger();
	virtual int DoJsonArray(Json::Value &dataValue);

	int PushTelemetry();
	int PushTelemetry(Json::Value &jsonValue);
	int PushAttributes();
	int PushAttributes(Json::Value &jsonValue);

	static void InitDeviceModelList();
	static void RegisterDeviceModel(uint32_t type, string model, string name);
	static uint32_t ConvertModelToDeviceType(string model);
	static string ConvertDeviceTypeToName(uint32_t type);

	virtual int BuildTelemetryValue(Json::Value &pushDataValue) { return CODE_ERROR; }
	// virtual void Getstatus(Json::Value &jsonValue) {}

	virtual void InputData(Json::Value &dataValue) {}
	virtual void InputData(uint8_t *data, int len, uint32_t addr = 0){};
	virtual bool CheckData(Json::Value &dataValue, bool &rs) { return false; }

	virtual int Do(Json::Value &dataValue) { return CODE_ERROR; }
};
