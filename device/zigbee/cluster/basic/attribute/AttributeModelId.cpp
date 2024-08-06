#include "AttributeModelId.h"
#include "../ClusterBasic.h"
#include "Util.h"
#include "Device.h"
#include "ZigbeeProtocol.h"

AttributeModelId::AttributeModelId(Cluster *cluster, string modelIdKey) : Attribute(ATTRIBUTE_BASIC_ModelIdentifier, cluster)
{
	this->modelIdKey = modelIdKey;
}

int AttributeModelId::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	return CODE_ERROR;
}

int AttributeModelId::InputData(uint8_t *data, int len, Json::Value &jsonValue, int *lenRemain)
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
			if (attributeMessage->dataType == ZCL_DATA_TYPE_CHAR_STR)
			{
				if (lenRemain)
				{
					*lenRemain = len - sizeof(AttributeMessage_st) - getSizeOfDataType(&attributeMessage->dataType);
					model = "";
					for (int j = 1; j <= attributeMessage->data[0]; j++)
					{
						model += attributeMessage->data[j];
					}
					LOGI("model: %s", model.c_str());
					BuildTelemetryValue(jsonValue);
					CheckTrigger();
					return CODE_OK;
				}
			}
		}
	}
	return CODE_ERROR;
}

void AttributeModelId::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[modelIdKey] = model;
}