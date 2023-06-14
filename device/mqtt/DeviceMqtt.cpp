#include "DeviceMqtt.h"
#include "Log.h"
#include "Db.h"
#include "function/FunctionZone.h"
#include "function/FunctionFace.h"

DeviceMqtt::DeviceMqtt(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version) : Device(id, name, mac, data, addr, type, version)
{
	protocol = MQTT_DEVICE;

	// parse data to function list
	Json::Value dataValue;
	if (dataValue.parse(data) && dataValue.isObject() &&
			dataValue.isMember("functions") && dataValue["functions"].isArray())
	{
		Json::Value functionsValue = dataValue["functions"];
		for (auto functionValue : functionsValue)
		{
			AddFuntion(functionValue, false);
		}
	}
	else
	{
		data = dataValue.toString();
		database->DeviceUpdateData(this);
	}
}

DeviceMqtt::~DeviceMqtt()
{
	for (auto &function : functions)
		delete function;
}

void DeviceMqtt::AddFuntion(Json::Value &dataValue, bool addToDb)
{
	if (dataValue.isMember("type") && dataValue["type"].isString() &&
			dataValue.isMember("id") && dataValue["id"].isString())
	{
		string id = dataValue["id"].asString();
		string type = dataValue["type"].asString();
		if (type == "Zone")
		{
			FunctionZone *functionZone = new FunctionZone(this, id);
			functions.push_back(functionZone);
		}
		else if (type == "Face")
		{
			FunctionFace *functionFace = new FunctionFace(this, id);
			functions.push_back(functionFace);
		}
		else
		{
			LOGW("Function type %s not support", type.c_str());
		}
	}
	if (addToDb)
	{
		Json::Value dataValue;
		if (dataValue.parse(data) && dataValue.isArray())
		{
			dataValue.append(dataValue);
			data = dataValue.toString();
			database->DeviceUpdateData(this);
		}
	}
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
