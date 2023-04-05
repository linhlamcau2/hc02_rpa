#pragma once

#include "Device.h"

using namespace std;

class DeviceBle : public Device
{
private:
	string deviceKey;

protected:
	int countElement;
	vector<Module *> modules;
	vector<Element *> elements;

public:
	DeviceBle(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
	string GetDeviceKey();

	virtual bool CheckAddr(uint32_t addr);

	virtual int BuildTelemetryValue(Json::Value &pushDataValue);
	virtual int BuildTelemetryValueV2(Json::Value &pushDataValue);

	virtual void InputData(Json::Value &dataValue);
	virtual void InputData(uint8_t *data, int len, uint32_t addr = 0);
	virtual bool CheckData(Json::Value &dataValue, bool &rs);

	virtual int Do(Json::Value &dataValue);
	virtual int DoV2(Json::Value &dataValue);
};
