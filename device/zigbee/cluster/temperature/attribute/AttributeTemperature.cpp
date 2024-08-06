#include "AttributeTemperature.h"
#include "../ClusterTemperature.h"
#include "Util.h"
#include "Device.h"
#include "ZigbeeProtocol.h"

AttributeTemperature::AttributeTemperature(Cluster *cluster, string temperatureKey) : Attribute(ATTRIBUTE_TEMPERATURE, cluster)
{
	this->temperatureKey = temperatureKey;
}

int AttributeTemperature::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	return CODE_ERROR;
}

int AttributeTemperature::InputData(uint8_t *data, int len, Json::Value &jsonValue, int *lenRemain)
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
			if (attributeMessage->dataType == ZCL_DATA_TYPE_INT16)
			{
				if (lenRemain)
				{
					*lenRemain = len - sizeof(AttributeMessage_st) - getSizeOfDataType(&attributeMessage->dataType);
					int16_t *data = (int16_t*)attributeMessage->data;
					temperature = bswap_16(data[0]);
					BuildTelemetryValue(jsonValue);
					CheckTrigger();
					return CODE_OK;
				}
			}
		}
	}
	return CODE_ERROR;
}

bool AttributeTemperature::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(temperatureKey) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[temperatureKey].isInt())
		{
			int temperature = dataValue[temperatureKey].asInt();
			rs = Util::CompareNumber(op, this->temperature, temperature);
			return true;
		}
		else if (dataValue[temperatureKey].isArray())
		{
			Json::Value listValue = dataValue[temperatureKey];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int temperature1 = listValue[0].asInt();
				int temperature2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->temperature, temperature1, temperature2);
				return true;
			}
		}
	}
	return false;
}

void AttributeTemperature::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[temperatureKey] = temperature;
}
