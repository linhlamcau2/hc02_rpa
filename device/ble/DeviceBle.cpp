#include "DeviceBle.h"
#include "Log.h"

DeviceBle::DeviceBle(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version) : Device(id, name, mac, data, addr, type, version)
{
	Json::Value dataJson;
	Json::Reader r;
	r.parse(data, dataJson);
	if (dataJson.isObject())
	{
		if (dataJson.isMember("devicekey") && dataJson["devicekey"].isString())
		{
			deviceKey = dataJson["devicekey"].asString();
		}
	}
	protocol = BLE_DEVICE;
	countElement = 1;
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

int DeviceBle::BuildTelemetryValueV2(Json::Value &pushDataValue)
{
	for (auto &module : modules)
	{
		module->BuildTelemetryValueV2(pushDataValue);
	}
	for (auto &element : elements)
	{
		element->BuildTelemetryValueV2(pushDataValue);
	}
	return CODE_OK;
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
		if (module->CheckData(dataValue, rs) == CODE_OK)
			return true;
	}
	for (auto &element : elements)
	{
		if (element->CheckData(dataValue, rs) == CODE_OK)
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

int DeviceBle::DoV2(Json::Value &dataValue)
{
	for (auto &module : modules)
	{
		module->DoV2(dataValue);
	}
	for (auto &element : elements)
	{
		element->DoV2(dataValue);
	}
	return CODE_OK;
}