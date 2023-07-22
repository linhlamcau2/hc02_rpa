#include "DeviceZigbee.h"
#include "Log.h"

DeviceZigbee::DeviceZigbee(string id, string name, string mac, uint32_t addr, uint32_t type) : Device(id, name, mac, "", addr, type, 0)
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
	values = Json::Value::null;
	for (auto &cluster : clusters)
	{
		if (cluster->InputData(data, len, values) == CODE_OK)
			break;
	}
	PushTelemetry(values);
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