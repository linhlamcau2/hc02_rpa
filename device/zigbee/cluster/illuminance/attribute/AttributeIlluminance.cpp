#include "AttributeIlluminance.h"
#include "../ClusterIlluminance.h"
#include "Util.h"
#include "Device.h"
#include "ZigbeeProtocol.h"

AttributeIlluminance::AttributeIlluminance(Cluster *cluster, string illuminanceKey) : Attribute(ATTRIBUTE_ILLUMINANCE, cluster)
{
	this->illuminanceKey = illuminanceKey;
}

int AttributeIlluminance::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	return CODE_ERROR;
}

int AttributeIlluminance::InputData(uint8_t *data, int len, Json::Value &jsonValue, int *lenRemain)
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
			if (attributeMessage->dataType == ZCL_DATA_TYPE_UINT16)
			{
				if (lenRemain)
				{
					*lenRemain = len - sizeof(AttributeMessage_st) - getSizeOfDataType(&attributeMessage->dataType);
					int16_t *data = (int16_t*)attributeMessage->data;
					illuminance = bswap_16(data[0]);
					BuildTelemetryValue(jsonValue);
					CheckTrigger();
					return CODE_OK;
				}
			}
		}
	}
	return CODE_ERROR;
}

bool AttributeIlluminance::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(illuminanceKey) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[illuminanceKey].isInt())
		{
			int illuminance = dataValue[illuminanceKey].asInt();
			rs = Util::CompareNumber(op, this->illuminance, illuminance);
			return true;
		}
		else if (dataValue[illuminanceKey].isArray())
		{
			Json::Value listValue = dataValue[illuminanceKey];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int illuminance1 = listValue[0].asInt();
				int illuminance2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->illuminance, illuminance1, illuminance2);
				return true;
			}
		}
	}
	return false;
}

void AttributeIlluminance::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[illuminanceKey] = illuminance;
}
