#pragma once

#include <string>
#include <vector>
#include <json.h>
#include <byteswap.h>
#include <RuleInputDevice.h>

using namespace std;

typedef enum
{
	BLE_DEVICE,
	ZIGBEE_DEVICE
} protocol_e;

enum
{
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
	BLE_LED_DAY_RGB = 13001,
	BLE_LED_DAY_RGBCW = 14001,
	BLE_LED_BULB = 14002,
	BLE_DOWNLIGHT_RGBCW = 14003,
	BLE_LED_OP_TRAN_LOA = 15001,
	BLE_SWITCH_4 = 22004,					// 0x020204,
	BLE_DC_SCENE_CONTACT = 23001, // 0x020301,
	BLE_TEMP_HUM_SENSOR = 38001,	// 0x030801,

	ZIGBEE_LUMI_PLUG = 0x02000001,
	ZIGBEE_LUMI_SENSOR_SWITCH = 0x02000002,
	ZIGBEE_PIR_RH3040 = 0x02000102,
	ZIGBEE_TELINK_TLSR82xx = 0x02000201
};

class Device
{
protected:
	string id;
	string name;
	string mac;
	uint32_t addr;
	uint32_t type;
	uint16_t version;
	int rssi;
	protocol_e protocol;
	string device_id;

public:
	vector<RuleInputDevice *> deviceRuleInputList;

public:
	Device(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version);
	virtual ~Device();

	string GetId();
	string GetName();
	string GetMac();
	uint32_t GetAddr();
	virtual bool CheckAddr(uint32_t addr);
	void SetAddr(uint32_t addr);
	uint32_t GetType();
	uint16_t GetVersion();
	string GetVersionStr();
	string GetDeviceId();
	int GetRSSI();

	void SetRSSI(int rssi);

	protocol_e GetProtocol();

	void RegisterTrigger(RuleInputDevice *ruleInputDevice);
	void UnregisterTrigger(RuleInputDevice *ruleInputDevice);

	virtual int BuildTelemetryValue(Json::Value &pushDataValue);
	virtual int BuildAttributesValue(Json::Value &pushDataValue);

	virtual void InitAttribute(int attributeId, double value) {}
	virtual void InputData(uint8_t *data, int len, uint32_t addr = 0) {}
	virtual bool CheckData(Json::Value &dataValue, bool &rs) { return false; }
	virtual void CheckTrigger();
	virtual bool Do(Json::Value &dataValue) { return false; }
	virtual bool DoJsonArray(Json::Value &dataValue);

	int PushTelemetry();
	int PushTelemetry(Json::Value jsonValue);
	int PushAttributes();
	int PushAttributes(Json::Value jsonValue);

	static void InitDeviceModelList();
	static void RegisterDeviceModel(uint32_t type, string model, string name);
	static uint32_t ConvertModelToDeviceType(string model);
	static string ConvertDeviceTypeToName(uint32_t type);
};
