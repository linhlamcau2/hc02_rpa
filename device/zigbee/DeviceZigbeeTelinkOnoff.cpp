#include "DeviceZigbeeTelinkOnoff.h"
#include "Log.h"
#include "ZigbeeProtocol.h"

DeviceZigbeeTelinkOnoff::DeviceZigbeeTelinkOnoff(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_TELINK_TLSR82xx)
{
	clusterOnoff = new ClusterOnoff(this, 1);
}

int DeviceZigbeeTelinkOnoff::BuildTelemetryValue(Json::Value &pushDataValue)
{
	clusterOnoff->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceZigbeeTelinkOnoff::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	uint16_t clusterId = (data[0] << 8) | data[1];
	LOGD("clusterId: 0x%04X", clusterId);
	if (clusterId == CLUSTER_ONOFF)
	{
		clusterOnoff->ParseData(data + 2, len, values);
	}
	else
	{
		LOGW("Cluster 0x%04X not handle", clusterId);
	}
	if (values != Json::Value::null)
	{
		PushTelemetry(values);
	}
}

bool DeviceZigbeeTelinkOnoff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (clusterOnoff->CheckData(dataValue, rs))
		return rs;
	return false;
}

bool DeviceZigbeeTelinkOnoff::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	if (dataValue.isMember("method") && dataValue["method"].isString())
	{
		string method = dataValue["method"].asString();
		if (method == "TurnOn")
		{
			zigbeeProtocol->ZCLOnoffDevice(addr, 0);
			return true;
		}
		else if (method == "TurnOff")
		{
			zigbeeProtocol->ZCLOnoffDevice(addr, 1);
			return true;
		}
		else if (method == "Toggle")
		{
			zigbeeProtocol->ZCLOnoffDevice(addr, 2);
			return true;
		}
		else
		{
			LOGW("DeviceZigbeeTelinkOnoff not handle method %s", method.c_str());
		}
	}
	return false;
}
