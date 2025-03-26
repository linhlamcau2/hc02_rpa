#pragma once

#include <string>
#include <vector>
#include <byteswap.h>
#include "json.h"
#include "ErrorCode.h"
#include "Object.h"

using namespace std;

class Device : public Object
{
protected:
	string mac;
	uint32_t type;
	uint16_t version;
	int rssi;
	bool isFavorite;
	Json::Value dataJson;
	Json::Value values;
	Json::Value propertyJsonUpdate;

public:

	int powerSource;
	bool lastOnlineState;
	time_t lastTimeActive;
	time_t lastTimeCheckActive;

public:
	Device(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version);
	virtual ~Device();

	string GetMac();
	uint32_t GetType();
	uint16_t GetVersion();
	string GetVersionStr();
	Json::Value GetData();
	int GetRSSI();

	void SetRSSI(int rssi);
	virtual bool CheckAddr(uint16_t addr) { return this->addr == addr; }
	// virtual bool CheckId(string id) { return this->id == id; }
	virtual string GetDeviceKey();

	bool isOnline();
	bool isNeedCheckOnline();
	void UpdateLastTimeActive();

	bool GetIsFavorite();
	bool SetIsFavorite(bool isFavorite);

	virtual int BuildAttributesValue(Json::Value &pushDataValue);

	void DeviceInputData(uint8_t *data, int len, uint16_t addr);

	virtual int InitAttribute(string attribute, double value) { return CODE_ERROR; }
	virtual void CheckTrigger();
	virtual int DoJsonArray(Json::Value &dataValue);

	virtual void SetPropertyJsonUpdate(Json::Value property);
	virtual Json::Value GetPropertyJsonUpdate();
	virtual void UpdatePropertyJsonUpdate(Json::Value &property);

	int PushTelemetry();
	int PushTelemetry(Json::Value &jsonValue);
	int PushAttributes();
	int PushAttributes(Json::Value &jsonValue);

	static void InitDeviceModelList();
	static void RegisterDeviceModel(uint32_t type, string model, const char *name);
	static uint32_t BleTypeToGroupId(uint32_t deviceType);
	static const char *BleAttributeIdToAttributeStr(uint16_t attributeId);
	// static uint32_t ConvertModelToDeviceType(string model);
	static const char *ConvertDeviceTypeToName(uint32_t type);
	static uint32_t ConverPidToDeviveType(uint16_t pid);

	virtual int BuildTelemetryValue(Json::Value &pushDataValue) { return CODE_ERROR; }
	// virtual void Getstatus(Json::Value &jsonValue) {}

	virtual void InputData(Json::Value &dataValue) {}
	virtual void InputData(uint8_t *data, int len, uint16_t addr = 0) {};
	virtual bool CheckData(Json::Value &dataValue, bool &rs) { return false; }
	virtual int GetNumElement() { return 1; }
	virtual int Do(Json::Value &dataValue) { return CODE_ERROR; }
};
