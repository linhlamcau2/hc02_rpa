#pragma once

#include <string>
#include <vector>
#include <json.h>
#include <byteswap.h>
#include <SceneInputDevice.h>

using namespace std;

typedef enum
{
	LORA_DEVICE,
	BLE_DEVICE,
	ZIGBEE_DEVICE,
	MODBUS_DEVICE
} protocol_e;

enum
{
	LORA_SENSOR_HUM_TEMP = 0,
	NONE,
	LORA_SENSOR_SOIL_MOISTURE,
	LORA_SENSOR_LIGHT,
	LORA_SENSOR_CO2,
	LORA_SENSOR_EC,
	LORA_SENSOR_PH,
	LORA_SENSOR_RAIN,

	BLE_DOWNLIGHT_SMT = 12001,		// 0x010201,
	BLE_SWITCH_4 = 22004,					// 0x020204,
	BLE_DC_SCENE_CONTACT = 23001, // 0x020301,
	BLE_TEMP_HUM_SENSOR = 38001,	// 0x030801,

	ZIGBEE_LUMI_PLUG = 0x02000001,
	ZIGBEE_LUMI_SENSOR_SWITCH = 0x02000002,
	ZIGBEE_PIR_RH3040 = 0x02000102,
	ZIGBEE_TELINK_TLSR82xx = 0x02000201,

	MODBUS_TEST = 0x03000000,
	MODBUS_ES_SM_TH_01 = 0x03000001,
};

#ifdef CONFIG_FPT_SERVER
extern map<string, int> parameterToId;
#endif

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

public:
	vector<SceneInputDevice *> deviceSceneInputList;

public:
	Device(string id, string name, string mac, uint32_t addr, uint32_t type);
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
	int GetRSSI();

	void SetRSSI(int rssi);

	protocol_e GetProtocol();

	int Online();
	int Offline();

	void RegisterTrigger(SceneInputDevice *sceneInputDevice);
	void UnregisterTrigger(SceneInputDevice *sceneInputDevice);

	virtual int BuildTelemetryValue(Json::Value &pushDataValue);
	virtual int BuildAttributesValue(Json::Value &pushDataValue);

	virtual void InitAttribute(int attributeId, double value) {}
	virtual void InputData(uint8_t *data, int len, uint32_t addr = 0) {}
	virtual bool CheckData(Json::Value &dataValue, bool &rs) { return false; }
	virtual void CheckTrigger();
	virtual bool Do(Json::Value &dataValue) { return false; }
	virtual bool Do(int id, int value) { return false; }

	int PushTelemetry();
	int PushTelemetry(Json::Value jsonValue);
	int PushAttributes();
	int PushAttributes(Json::Value jsonValue);

	static void InitDeviceModelList();
	static void RegisterDeviceModel(uint32_t type, string model, string name);
	static uint32_t ConvertModelToDeviceType(string model);
	static string ConvertDeviceTypeToName(uint32_t type);
};
