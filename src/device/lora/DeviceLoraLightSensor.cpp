#include "DeviceLoraLightSensor.h"
#include <Log.h>
#include <Util.h>

DeviceLoraLightSensor::DeviceLoraLightSensor(string id, string name, string mac, uint32_t addr) : DeviceLora(id, name, mac, addr, LORA_SENSOR_LIGHT)
{
	lux = 0;
	pin = 0;
}

int DeviceLoraLightSensor::BuildTelemetryValue(Json::Value &pushDataValue)
{
	pushDataValue["lux"] = lux;
	pushDataValue["pin"] = pin;
	return 0;
}

void DeviceLoraLightSensor::InputData(uint8_t *data, int len, uint32_t addr)
{
	light_sensor_st *light_sensor = (light_sensor_st *)data;
	lux = bswap_32(light_sensor->lux);
	pin = light_sensor->pin;
	CheckTrigger();
	PushTelemetry();
}

bool DeviceLoraLightSensor::CheckData(Json::Value &dataValue, bool& rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember("lux") && dataValue["lux"].isInt())
		{
			int lux = dataValue["lux"].asInt();
			rs = Util::CompareNumber(this->lux, lux, op);
			return true;
		}
		else if (dataValue.isMember("pin") && dataValue["pin"].isInt())
		{
			int pin = dataValue["pin"].asInt();
			rs = Util::CompareNumber(this->pin, pin, op);
			return true;
		}
	}
	return false;
}

bool DeviceLoraLightSensor::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}
