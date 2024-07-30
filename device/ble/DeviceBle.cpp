#include "DeviceBle.h"
#include "Log.h"

DeviceBle::DeviceBle(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version) : Device(id, name, mac, dataJson, addr, type, version)
{
	protocol = BLE_DEVICE;
	countElement = 1;
	deviceKey = GetDeviceKey(dataJson);
}

DeviceBle::~DeviceBle()
{
	for (auto &module : modules)
		delete module;
}

string DeviceBle::GetDeviceKey(Json::Value &dataJson)
{
	if (dataJson.isObject())
	{
		if (dataJson.isMember("devicekey") && dataJson["devicekey"].isString())
		{
			deviceKey = dataJson["devicekey"].asString();
		}
	}
	return deviceKey;
}

bool DeviceBle::CheckAddr(uint16_t addr)
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
	int temp = isOnline();
	pushDataValue["stt"] = temp;
	return CODE_OK;
}

void DeviceBle::InputData(Json::Value &dataValue, bool isPushTelemety)
{
	values = Json::Value::null;
	for (auto &module : modules)
	{
		module->InputData(dataValue, values);
	}
	if (!values.isNull() && isPushTelemety)
	{
		Json::Value dataJson;
		dataJson["attribute"] = "device.status";
		dataJson["mac"] = mac,
		dataJson["value"] = values;
		PushTelemetry(dataJson);
	}
}

void DeviceBle::InputData(uint8_t *data, int len, uint16_t addr)
{
	values = Json::Value::null;
	for (auto &module : modules)
	{
		if (module->CheckAddr(addr))
		{
			if (module->InputData(data, len, values) == CODE_OK)
				break;
		}
	}
	if (!values.isNull())
	{
		Json::Value dataJson;
		dataJson["attribute"] = "device.status";
		dataJson["mac"] = mac,
		dataJson["value"] = values;
		PushTelemetry(dataJson);
	}
}

bool DeviceBle::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	for (auto &module : modules)
	{
		if (module->CheckData(dataValue, rs))
			return true;
	}
	return false;
}

int DeviceBle::GetNumElement()
{
	return countElement;
}

int DeviceBle::Do(Json::Value &dataValue)
{
	for (auto &module : modules)
	{
		module->Do(dataValue);
	}
	return CODE_OK;
}

int DeviceBle::InitAttribute(string attribute, int value)
{
	for (auto &module : modules)
	{
		module->InitAttribute(attribute, value);
	}
	return CODE_OK;
}
