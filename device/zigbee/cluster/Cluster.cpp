#include "Cluster.h"
#include <byteswap.h>
#include "ZigbeeDataTypes.h"

Cluster::Cluster(uint16_t id, Device *device, uint8_t endpoint)
{
	this->id = id;
	this->device = device;
	this->endpoint = endpoint;
}

void Cluster::BuildTelemetryValue(Json::Value &pushDataValue)
{
	for (auto &attribute : attributes)
	{
		attribute->BuildTelemetryValue(pushDataValue);
	}
}

int Cluster::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	for (auto &attribute : attributes)
	{
		attribute->InputData(dataValue, jsonValue);
	}
	return CODE_OK;
}

int getSizeOfDataType(uint8_t *dataType)
{
	switch (dataType[0])
	{
	case ZIGBEE_DATATYPE_BOOL:
	case ZIGBEE_DATATYPE_UINT8:
	case ZIGBEE_DATATYPE_ENUM8:
		return 1;

	case ZIGBEE_DATATYPE_UINT16:
	case ZIGBEE_DATATYPE_INT16:
		return 2;

	case ZIGBEE_DATATYPE_STRING:
		return dataType[1] + 1;

	case ZIGBEE_DATATYPE_STRUCT:
	{
		typedef struct __attribute__((packed))
		{
			uint16_t numberOfElement;
			uint8_t data[];
		} DataType_st;
		DataType_st *dataStruct = (DataType_st *)dataType;
		uint8_t *data = dataStruct->data;
		int dataLen = 2;
		int dataSize = 0;
		for (int i = 0; i < dataStruct->numberOfElement; i++)
		{
			dataSize = getSizeOfDataType(data);
			dataLen += 1 + dataSize;
			data += 1 + dataSize;
		}
		LOGW("Data struct: %d", dataSize);
		return dataLen;
	}

	default:
		LOGW("getSizeOfDataType not check type 0x%02X", dataType[0]);
		return 0;
	}
}

int Cluster::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint16_t clusterId;
		uint8_t attrNum;
	} ClusterMessage_st;
	ClusterMessage_st *clusterMessage = (ClusterMessage_st *)data;

	typedef struct __attribute__((packed))
	{
		uint16_t attrID;
		uint8_t dataType;
		uint8_t data[];
	} AttributeMessage_st;
	AttributeMessage_st *attributeMessage = NULL;

	if (len > sizeof(ClusterMessage_st))
	{
		uint8_t *attrData = data + 3;
		int attrLen = len - 3;
		int lenRemain = 0;
		int oldLen;
		if (bswap_16(clusterMessage->clusterId) == id)
		{
			LOGD("clusterMessage->clusterId: 0x%04X, clusterMessage->attrNum: %d", bswap_16(clusterMessage->clusterId), clusterMessage->attrNum);
			for (int i = 0; i < clusterMessage->attrNum; i++)
			{
				oldLen = attrLen;
				for (auto &attribute : attributes)
				{
					if (attribute->InputData(attrData, attrLen, jsonValue, &lenRemain) == CODE_OK)
					{
						attrData += attrLen - lenRemain;
						attrLen = lenRemain;
						break;
					}
				}
				if (oldLen == attrLen)
				{
					attributeMessage = (AttributeMessage_st *)attrData;
					LOGW("Attribute ID: 0x%04X not handle", bswap_16(attributeMessage->attrID));
					int dataSize = getSizeOfDataType(&attributeMessage->dataType);
					attrData += 3 + dataSize;
					attrLen -= 3 + dataSize;
				}
			}
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

bool Cluster::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	for (auto &attribute : attributes)
	{
		if (attribute->CheckData(dataValue, rs))
			return true;
	}
	return false;
}

int Cluster::Do(Json::Value &dataValue)
{
	for (auto &attribute : attributes)
	{
		attribute->Do(dataValue);
	}
	return CODE_OK;
}
