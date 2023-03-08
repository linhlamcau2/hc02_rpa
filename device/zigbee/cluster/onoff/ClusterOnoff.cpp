#include "ClusterOnoff.h"
#include "Log.h"

ClusterOnoff::ClusterOnoff(Device *device, uint8_t endpoint) : Cluster(device, endpoint)
{
	attributeOnoff = new AttributeOnoff(this);
}

void ClusterOnoff::InitAttribute(int attributeId, double value)
{
	attributeOnoff->InitAttribute(attributeId, value);
}

void ClusterOnoff::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
	uint8_t attrNum = data[0];
	int lenRemain = len;
	uint8_t *messageAttr = data + 1;
	uint16_t attrID;
	int dataLen;
	for (int i = 0; i < attrNum; i++)
	{
		if (lenRemain < 4)
			break;
		attrID = (messageAttr[0] << 8) | messageAttr[1];
		dataLen = 1;
		LOGD("Attribute ID: 0x%04X", attrID);
		if (attrID == ATTRIBUTE_ONOFF)
		{
			attributeOnoff->ParseData(messageAttr + 2, lenRemain - 2, jsonValue);
		}
		else
		{
			LOGW("Attribute 0x%04X not handle", attrID);
		}
		lenRemain -= 3 + dataLen;
		messageAttr += 3 + dataLen;
	}
}

bool ClusterOnoff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (attributeOnoff->CheckData(dataValue, rs))
		return rs;
	return false;
}

void ClusterOnoff::BuildTelemetryValue(Json::Value &jsonValue)
{
	attributeOnoff->BuildTelemetryValue(jsonValue);
}
