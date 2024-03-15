#pragma once

#include "Device.h"
#include "module/Module.h"

using namespace std;

enum
{
	BLE_DOWNLIGHT_SMT_GROUP = 1,
	BLE_DOWNLIGHT_COB_GOC_RONG_GROUP = 2,
	BLE_DOWNLIGHT_COB_GOC_HEP_GROUP = 3,
	BLE_DOWNLIGHT_COB_TRANG_TRI_GROUP = 4,
	BLE_PANEL_TRON_GROUP = 5,
	BLE_PANEL_VUONG_GROUP = 6,
	BLE_LED_OP_TRAN_GROUP = 7,
	BLE_LED_OP_TUONG_GROUP = 8,
	BLE_LED_CHIEU_TRANH_GROUP = 9,
	BLE_TRACKLIGHT_GROUP = 10,
	BLE_LED_THA_TRAN_GROUP = 11,
	BLE_LED_CHIEU_GUONG_GROUP = 12,
	BLE_LED_DAY_LINEAR_GROUP = 13,
	BLE_LED_TUBE_M16_GROUP = 14,
	BLE_DEN_BAN_GROUP = 15,
	BLE_LED_FLOOD_GROUP = 16,
	BLE_LED_DAY_RGB_GROUP = 17,
	BLE_LED_DAY_RGBCW_GROUP = 18,
	BLE_LED_BULB_GROUP = 19,
	BLE_DOWNLIGHT_RGBCW_GROUP = 20,
	BLE_LED_OP_TRAN_LOA_GROUP = 21,
	BLE_LED_RLT03_06W_GROUP = 23,
	BLE_LED_RLT02_10W_GROUP = 24,
	BLE_LED_RLT02_20W_GROUP = 25,
	BLE_LED_RLT01_10W_GROUP = 26,
	BLE_LED_TRL08_20W_GROUP = 27,
	BLE_LED_TRL08_10W_GROUP = 28,
	BLE_LED_RLT03_12W_GROUP = 29,
	BLE_SWITCH_ONOFF_V2_GROUP = 30,
	BLE_SWITCH_TOUCH_GROUP = 31,
	BLE_SWITCH_ELECTRICAL_GROUP = 32,
	BLE_SWITCH_CEILING_GROUP = 33,
	BLE_SWITCH_CURTAIN_GROUP = 34,
};

class DeviceBle : public Device
{
private:
	string deviceKey;

protected:
	int countElement;
	vector<Module *> modules;

public:
	DeviceBle(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
	~DeviceBle();

	string GetDeviceKey(Json::Value &dataJson);
	string GetDeviceKey();

	virtual bool CheckAddr(uint16_t addr);

	virtual int BuildTelemetryValue(Json::Value &pushDataValue);

	// virtual void Getstatus(Json::Value &jsonValue);

	virtual void InputData(Json::Value &dataValue, bool isPushTelemety = true);
	virtual void InputData(uint8_t *data, int len, uint16_t addr = 0);
	virtual bool CheckData(Json::Value &dataValue, bool &rs);
	int GetNumElement();

	virtual int Do(Json::Value &dataValue);
};
