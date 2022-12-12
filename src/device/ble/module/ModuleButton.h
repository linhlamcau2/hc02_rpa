#pragma once
#include "Module.h"

using namespace std;

class ModuleButton : public Module
{
protected:
	uint8_t bt;
	int index;
	string buttonName;

public:
	ModuleButton(Device *device, int index);

#ifdef CONFIG_FPT_SERVER
	void InitAttribute(int attributeId, double value);
	void SaveAttribute();
#endif

	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
	bool CheckData(Json::Value dataValue);
	void CheckTrigger();
	void BuildTelemetryValue(Json::Value &jsonValue);
};
