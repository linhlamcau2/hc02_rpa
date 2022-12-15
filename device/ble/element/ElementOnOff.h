#pragma once
#include "Element.h"

using namespace std;

class ElementOnOff : public Element
{
protected:
	uint8_t onoff;
	string elementName;

public:
	ElementOnOff(Device *device, uint32_t addr);

	void InitAttribute(int attributeId, double value);
	void SaveAttribute();
	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
	bool CheckData(Json::Value &dataValue, bool &rs);

	void CheckTrigger();
	void BuildTelemetryValue(Json::Value &jsonValue);
	bool Do(Json::Value &dataValue);
	bool Do(int value);
};
