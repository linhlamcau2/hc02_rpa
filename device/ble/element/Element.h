#pragma once

#include <string>
#include <json.h>

using namespace std;

class Device;
class Element
{
protected:
	uint32_t addr;
	Device *device;

public:
	Element(Device *device, uint32_t addr);

	virtual void BuildTelemetryValue(Json::Value &jsonValue) {}
	virtual bool Do(Json::Value &dataValue) { return false; }
};
