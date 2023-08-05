#include "DeviceZigbee.h"
#include "Log.h"

DeviceZigbee::DeviceZigbee(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type) : Device(id, name, mac, dataJson, addr, type, 0)
{
	protocol = ZIGBEE_DEVICE;
}

int DeviceZigbee::BuildTelemetryValue(Json::Value &pushDataValue)
{
	for (auto &cluster : clusters)
	{
		cluster->BuildTelemetryValue(pushDataValue);
	}
	return CODE_OK;
}

void DeviceZigbee::InputData(Json::Value &dataValue)
{
	values = Json::Value::null;
	for (auto &cluster : clusters)
	{
		cluster->InputData(dataValue, values);
	}
	if (!values.isNull())
		PushTelemetry(values);
}

void DeviceZigbee::InputData(uint8_t *data, int len)
{
	int rs = CODE_ERROR;
	values = Json::Value::null;
	for (auto &cluster : clusters)
	{
		if (cluster->InputData(data, len, values) == CODE_OK)
		{
			rs = CODE_OK;
			break;
		}
	}
	if (rs == CODE_OK)
		PushTelemetry(values);
	else
	{
		typedef struct __attribute__((packed))
		{
			uint16_t clusterId;
			uint8_t attrNum;
		} ClusterMessage_st;
		ClusterMessage_st *clusterMessage = (ClusterMessage_st *)data;
		LOGW("Cluster id 0x%04X data not handle", bswap_16(clusterMessage->clusterId));
	}
}

bool DeviceZigbee::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	for (auto &cluster : clusters)
	{
		if (cluster->CheckData(dataValue, rs))
			return true;
	}
	return false;
}

int DeviceZigbee::Do(Json::Value &dataValue)
{
	for (auto &cluster : clusters)
	{
		cluster->Do(dataValue);
	}
	return CODE_OK;
}