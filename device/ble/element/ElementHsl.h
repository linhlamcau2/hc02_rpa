#pragma once
#include "Element.h"

using namespace std;

class ElementHsl : public Element
{
protected:
	uint16_t h;
    uint16_t s;
    uint16_t l;
    string elementNameH;
    string elementNameL;
    string elementNameS;
public:
	ElementHsl(Device *device, uint32_t addr);

	void InitAttribute(int attributeId, double value);
	void SaveAttribute();
	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
	bool CheckData(Json::Value &dataValue, bool &rs);

	void CheckTrigger();
	void BuildTelemetryValue(Json::Value &jsonValue);
	bool Do(Json::Value &dataValue);
	bool Do(uint16_t valueH, uint16_t valueS, uint16_t valueL);
};
