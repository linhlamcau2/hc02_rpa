#pragma once
#include "Element.h"

using namespace std;

class ElementResetNode : public Element
{
public:
	ElementResetNode(Device *device, uint32_t addr);
	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
	void BuildTelemetryValue(Json::Value &jsonValue);
	bool Do();
};
