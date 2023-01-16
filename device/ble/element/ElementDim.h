#pragma once
#include "Element.h"

using namespace std;

class ElementDim : public Element
{
protected:
	uint16_t dim;
    string elementName;

public:
	ElementDim(Device *device, uint32_t addr);

	void InitAttribute(int attributeId, double value);
	void SaveAttribute();
	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
	bool CheckData(Json::Value &dataValue, bool &rs);

	void CheckTrigger();
	void BuildTelemetryValue(Json::Value &jsonValue);
	bool Do(Json::Value &dataValue);
	bool Do(uint16_t value);
};
