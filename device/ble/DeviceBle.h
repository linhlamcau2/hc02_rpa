#pragma once

#include "Device.h"
#include "module/Module.h"

using namespace std;

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
	virtual int InitAttribute(string attribute, int value);
};
