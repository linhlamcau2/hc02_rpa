#pragma once
#include "zigbee/cluster/Attribute.h"

using namespace std;

class AttributeOnoff : public Attribute
{
private:
	uint8_t onoff;

public:
	AttributeOnoff(Cluster *cluster);

#ifdef CONFIG_FPT_SERVER
	void InitAttribute(int attributeId, double value);
	void SaveAttribute();
#endif
	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
	bool CheckData(Json::Value &dataValue, bool &rs);
	void BuildTelemetryValue(Json::Value &jsonValue);
};
