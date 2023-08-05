#include "DeviceMqtt.h"
#include "Log.h"
#include "Db.h"
#include "function/FunctionZone.h"
#include "function/FunctionFace.h"

DeviceMqtt::DeviceMqtt(string id, string name, string mac, Json::Value &dataJson, uint32_t addr, uint32_t type, uint16_t version) : Device(id, name, mac, dataJson, addr, type, version)
{
	protocol = MQTT_DEVICE;

	// parse data to function list
	if (dataJson.isObject() &&
			dataJson.isMember("functions") && dataJson["functions"].isArray())
	{
		Json::Value functionsValue = dataJson["functions"];
		for (auto functionValue : functionsValue)
		{
			AddFuntion(functionValue, false);
		}
	}
}

DeviceMqtt::~DeviceMqtt()
{
	for (auto &function : functions)
		delete function;
}

void DeviceMqtt::AddFuntion(Json::Value &funcValue, bool addToDb)
{
	if (funcValue.isMember("type") && funcValue["type"].isString() &&
			funcValue.isMember("id") && funcValue["id"].isString())
	{
		string id = funcValue["id"].asString();
		string type = funcValue["type"].asString();
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
		if (dataJson.isObject())
		{
			dataJson["functions"].append(funcValue);
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
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	for (auto &function : functions)
	{
		if (function->CheckData(dataValue, rs))
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
