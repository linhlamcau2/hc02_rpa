#include "DeviceZigbeeOnoff.h"
#include "Log.h"
#include "ZigbeeProtocol.h"

DeviceZigbeeOnoff::DeviceZigbeeOnoff(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_LUMI_PLUG)
{
	clusterOnoff = new ClusterOnoff(this, 1);
}

int DeviceZigbeeOnoff::BuildTelemetryValue(Json::Value &pushDataValue)
{
	clusterOnoff->BuildTelemetryValue(pushDataValue);
	return CODE_OK;
}

void DeviceZigbeeOnoff::InitAttribute(int attributeId, double value)
{
	clusterOnoff->InitAttribute(attributeId, value);
}

void DeviceZigbeeOnoff::InputData(uint8_t *data, int len, uint32_t addr)
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

bool DeviceZigbeeOnoff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (clusterOnoff->CheckData(dataValue, rs))
		return rs;
	return false;
}

int DeviceZigbeeOnoff::Do(Json::Value &dataValue)
{
	LOGD("Do data: %s", dataValue.toString().c_str());
	if (dataValue.isMember("method") && dataValue["method"].isString())
	{
		string method = dataValue["method"].asString();
		if (method == "TurnOn")
		{
			zigbeeProtocol->ZCLOnoffDevice(addr, 0);
			return CODE_OK;
		}
		else if (method == "TurnOff")
		{
			zigbeeProtocol->ZCLOnoffDevice(addr, 1);
			return CODE_OK;
		}
		else if (method == "Toggle")
		{
			zigbeeProtocol->ZCLOnoffDevice(addr, 2);
			return CODE_OK;
		}
		else
		{
			LOGW("DeviceZigbeeOnoff not handle method %s", method.c_str());
		}
	}
	return CODE_ERROR;
}
