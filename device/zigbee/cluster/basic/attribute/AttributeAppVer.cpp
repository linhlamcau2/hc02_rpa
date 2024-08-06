#include "AttributeAppVer.h"
#include "../ClusterBasic.h"
#include "Util.h"
#include "Device.h"
#include "ZigbeeProtocol.h"

AttributeAppVer::AttributeAppVer(Cluster *cluster, string appVerKey) : Attribute(ATTRIBUTE_BASIC_ApplicationVersion, cluster)
{
	this->appVerKey = appVerKey;
}

int AttributeAppVer::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	return CODE_ERROR;
}

int AttributeAppVer::InputData(uint8_t *data, int len, Json::Value &jsonValue, int *lenRemain)
{
	typedef struct __attribute__((packed))
	{
		uint16_t attrID;
		uint8_t dataType;
		uint8_t data[];
	} AttributeMessage_st;
	AttributeMessage_st *attributeMessage = (AttributeMessage_st *)data;
	if (len > sizeof(AttributeMessage_st))
	{
		if (bswap_16(attributeMessage->attrID) == id)
		{
			if (attributeMessage->dataType == ZCL_DATA_TYPE_UINT8)
			{
				appVer = attributeMessage->data[0];
				LOGI("appVer: 0x%02X", appVer);
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}

void AttributeAppVer::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[appVerKey] = appVer;
}