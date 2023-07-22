#include "DeviceZigbeeTelinkOnoff.h"
#include "Log.h"
#include "ZigbeeProtocol.h"

DeviceZigbeeTelinkOnoff::DeviceZigbeeTelinkOnoff(string id, string name, string mac, uint32_t addr)
		: DeviceZigbee(id, name, mac, addr, ZIGBEE_TELINK_TLSR82xx)
{
	clusterOnoff = new ClusterOnoff(this, 1);
	clusters.push_back(clusterOnoff);
}

int DeviceZigbeeTelinkOnoff::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
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
