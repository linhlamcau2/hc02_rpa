#include "DeviceZigbeeOnoff.h"
#include "Log.h"
#include "ZigbeeProtocol.h"

DeviceZigbeeOnoff::DeviceZigbeeOnoff(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_LUMI_PLUG)
{
	clusterOnoff = new ClusterOnoff(this, 1);
	clusters.push_back(clusterOnoff);
}

int DeviceZigbeeOnoff::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
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
