#include "AttributeOnoff.h"
#include "../ClusterOnoff.h"
#include "Util.h"
#include "Device.h"
#include "ZigbeeProtocol.h"

AttributeOnoff::AttributeOnoff(Cluster *cluster, string onoffKey) : Attribute(ATTRIBUTE_ONOFF, cluster)
{
	this->onoffKey = onoffKey;
}

int AttributeOnoff::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	return CODE_ERROR;
}

int AttributeOnoff::InputData(uint8_t *data, int len, Json::Value &jsonValue, int *lenRemain)
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
		if (bswap_16(attributeMessage->attrID) == ATTRIBUTE_ONOFF)
		{
			if (attributeMessage->dataType == 0x10)
			{
				if (lenRemain)
				{
					*lenRemain = len - sizeof(AttributeMessage_st) - getSizeOfDataType(&attributeMessage->dataType);
					onoff = attributeMessage->data[0];
					BuildTelemetryValue(jsonValue);
					CheckTrigger();
					return CODE_OK;
				}
			}
		}
	}
	return CODE_ERROR;
}

bool AttributeOnoff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(onoffKey) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[onoffKey].isInt())
		{
			int onoff = dataValue[onoffKey].asInt();
			rs = Util::CompareNumber(op, this->onoff, onoff);
			return true;
		}
		else if (dataValue[onoffKey].isArray())
		{
			Json::Value listValue = dataValue[onoffKey];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int onoff1 = listValue[0].asInt();
				int onoff2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->onoff, onoff1, onoff2);
				return true;
			}
		}
	}
	return false;
}

void AttributeOnoff::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[onoffKey] = onoff;
}

int AttributeOnoff::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (zigbeeProtocol && dataValue.isObject() &&
			dataValue.isMember(onoffKey) && dataValue[onoffKey].isInt())
	{
		int onoff = dataValue[onoffKey].asInt();
		LOGD("Do onoff: %d", onoff);
		if (zigbeeProtocol->ZCLOnoffDevice(cluster->getDevice()->GetAddr(), onoff) == CODE_OK)
		{
			this->onoff = onoff;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
