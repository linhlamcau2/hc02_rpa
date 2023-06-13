#include "DeviceMqtt.h"
#include "Log.h"

DeviceMqtt::DeviceMqtt(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version) : Device(id, name, mac, data, addr, type, version)
{
	protocol = MQTT_DEVICE;
}

DeviceMqtt::~DeviceMqtt()
{
	for (auto &function : functions)
		delete function;
}

void DeviceMqtt::AddFuntion(Function *function)
{
	functions.push_back(function);
}

int DeviceMqtt::BuildTelemetryValue(Json::Value &pushDataValue)
{
	for (auto &function : functions)
	{
		function->BuildTelemetryValue(pushDataValue);
	}
	return CODE_OK;
}

void DeviceMqtt::InputData(Json::Value &dataValue)
{
	values = Json::Value::null;
	for (auto &function : functions)
	{
		function->InputData(dataValue, values);
	}
	if (!values.isNull())
		PushTelemetry(values);
}

bool DeviceMqtt::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	for (auto &function : functions)
	{
		if (function->CheckData(dataValue, rs) == CODE_OK)
			return true;
	}
	return false;
}

int DeviceMqtt::Do(Json::Value &dataValue)
{
	for (auto &function : functions)
	{
		function->Do(dataValue);
	}
	return CODE_OK;
}
