#include "AttributeHumidity.h"
#include "../ClusterHumidity.h"
#include "Util.h"
#include "Device.h"
#include "ZigbeeProtocol.h"
#include "ZigbeeDataTypes.h"

AttributeHumidity::AttributeHumidity(Cluster *cluster, string humidityKey) : Attribute(ATTRIBUTE_HUMIDITY, cluster)
{
	this->humidityKey = humidityKey;
}

int AttributeHumidity::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	return CODE_ERROR;
}

int AttributeHumidity::InputData(uint8_t *data, int len, Json::Value &jsonValue, int *lenRemain)
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
		if (bswap_16(attributeMessage->attrID) == ATTRIBUTE_HUMIDITY)
		{
			if (attributeMessage->dataType == ZIGBEE_DATATYPE_UINT16)
			{
				if (lenRemain)
				{
					*lenRemain = len - sizeof(AttributeMessage_st) - getSizeOfDataType(&attributeMessage->dataType);
					int16_t *data = (int16_t*)attributeMessage->data;
					humidity = bswap_16(data[0]);
					BuildTelemetryValue(jsonValue);
					CheckTrigger();
					return CODE_OK;
				}
			}
		}
	}
	return CODE_ERROR;
}

bool AttributeHumidity::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(humidityKey) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[humidityKey].isInt())
		{
			int humidity = dataValue[humidityKey].asInt();
			rs = Util::CompareNumber(op, this->humidity, humidity);
			return true;
		}
		else if (dataValue[humidityKey].isArray())
		{
			Json::Value listValue = dataValue[humidityKey];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int humidity1 = listValue[0].asInt();
				int humidity2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->humidity, humidity1, humidity2);
				return true;
			}
		}
	}
	return false;
}

void AttributeHumidity::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[humidityKey] = humidity;
}
