#pragma once
#include "zigbee/cluster/Cluster.h"
// #include "attribute/AttributeOnoff.h"

#define ATTRIBUTE_BASIC_ZCLVersion 0x0000
#define ATTRIBUTE_BASIC_ApplicationVersion 0x0001
#define ATTRIBUTE_BASIC_StackVersion 0x0002
#define ATTRIBUTE_BASIC_HWVersion 0x0003
#define ATTRIBUTE_BASIC_ManufacturerName 0x0004
#define ATTRIBUTE_BASIC_ModelIdentifier 0x0005
#define ATTRIBUTE_BASIC_DateCode 0x0006
#define ATTRIBUTE_BASIC_PowerSource 0x0007

using namespace std;

class ClusterBasic : public Cluster
{
private:
	// AttributeOnoff *attributeOnoff;

protected:
public:
	ClusterBasic(Device *device);

	void ParseData(uint8_t *data, int len, Json::Value &jsonValue);
};
