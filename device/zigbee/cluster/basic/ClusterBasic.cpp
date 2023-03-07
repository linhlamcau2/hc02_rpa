#include "ClusterBasic.h"
#include "Log.h"

ClusterBasic::ClusterBasic(Device *device) : Cluster(device)
{
	// attributeOnoff = new AttributeOnoff(this);
}

void ClusterBasic::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
	// uint8_t attrNum = data[0];
	// int lenRemain = len;
	// uint8_t *messageAttr = data + 1;
	// uint16_t attrID;
	// int dataLen;
	// for (int i = 0; i < attrNum; i++)
	// {
	// 	if (lenRemain < 4)
	// 		break;
	// 	attrID = (messageAttr[0] << 8) | messageAttr[1];
	// 	dataLen = 1;
	// 	LOGD("Attribute ID: 0x%04X", attrID);
	// 	if (attrID == ATTRIBUTE_ONOFF)
	// 	{
	// 		attributeOnoff->ParseData(messageAttr + 2, lenRemain - 2, jsonValue);
	// 	}
	// 	else
	// 	{
	// 		LOGW("Attribute 0x%04X not handle", attrID);
	// 	}
	// 	lenRemain -= 3 + dataLen;
	// 	messageAttr += 3 + dataLen;
	// }
}
