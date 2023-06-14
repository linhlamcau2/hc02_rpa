#include "DeviceBle.h"
#include "Log.h"

DeviceBle::DeviceBle(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version) : Device(id, name, mac, data, addr, type, version)
{
	protocol = BLE_DEVICE;
	countElement = 1;
	deviceKey = GetDeviceKey(data);
}

DeviceBle::~DeviceBle()
{
	for (auto &module : modules)
		delete module;
	for (auto &element : elements)
		delete element;
}

string DeviceBle::GetDeviceKey(string data)
{
	Json::Value dataJson;
	if (dataJson.parse(data) && dataJson.isObject())
	{
		if (dataJson.isMember("devicekey") && dataJson["devicekey"].isString())
		{
			deviceKey = dataJson["devicekey"].asString();
		}
	}
	return deviceKey;
}

bool DeviceBle::CheckAddr(uint32_t addr)
{
	return ((this->addr <= addr) && (this->addr + countElement - 1 >= addr));
}

string DeviceBle::GetDeviceKey()
{
	return deviceKey;
}

int DeviceBle::BuildTelemetryValue(Json::Value &pushDataValue)
{
	for (auto &module : modules)
	{
		module->BuildTelemetryValue(pushDataValue);
	}
	for (auto &element : elements)
	{
		element->BuildTelemetryValue(pushDataValue);
	}
	return CODE_OK;
}

void DeviceBle::InputData(Json::Value &dataValue)
{
	values = Json::Value::null;
	for (auto &module : modules)
	{
		module->InputData(dataValue, values);
	}
	for (auto &element : elements)
	{
		element->InputData(dataValue, values);
	}
	if (!values.isNull())
		PushTelemetry(values);
}

void DeviceBle::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	for (auto &module : modules)
	{
		if (module->InputData(data, len, values) == CODE_OK)
			break;
	}
	for (auto &element : elements)
	{
		if (element->CheckAddr(addr))
		{
			if (element->InputData(data, len, values) == CODE_OK)
				break;
		}
	}
	PushTelemetry(values);
}

bool DeviceBle::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	for (auto &module : modules)
	{
		if (module->CheckData(dataValue, rs))
			return true;
	}
	for (auto &element : elements)
	{
		if (element->CheckData(dataValue, rs))
			return true;
	}
	return false;
}

int DeviceBle::Do(Json::Value &dataValue)
{
	for (auto &module : modules)
	{
		module->Do(dataValue);
	}
	for (auto &element : elements)
	{
		element->Do(dataValue);
	}
	return CODE_OK;
}
