#pragma once

#include "Device.h"
#include "function/Function.h"

using namespace std;

class DeviceMqtt : public Device
{
private:
protected:
	vector<Function *> functions;

public:
	DeviceMqtt(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version);
	~DeviceMqtt();

	void AddFuntion(Json::Value &dataValue, bool addToDb);
	virtual int BuildTelemetryValue(Json::Value &pushDataValue);
	virtual void InputData(Json::Value &dataValue);
	virtual bool CheckData(Json::Value &dataValue, bool &rs);
	virtual int Do(Json::Value &dataValue);
};
