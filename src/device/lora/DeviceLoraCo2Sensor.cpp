#include "DeviceLoraCo2Sensor.h"
#include <Log.h>
#include <Util.h>

DeviceLoraCo2Sensor::DeviceLoraCo2Sensor(string id, string name, string mac, uint32_t addr) : DeviceLora(id, name, mac, addr, LORA_SENSOR_CO2)
{
	co2 = 0;
	tovc = 0;
	pin = 0;
}

int DeviceLoraCo2Sensor::BuildTelemetryValue(Json::Value &pushDataValue)
{
	pushDataValue["co2"] = co2;
	pushDataValue["tovc"] = tovc;
	pushDataValue["pin"] = pin;
	return 0;
}

void DeviceLoraCo2Sensor::InputData(uint8_t *data, int len, uint32_t addr)
{
	co2_sensor_st *co2_sensor = (co2_sensor_st *)data;
	co2 = bswap_16(co2_sensor->co2);
	tovc = bswap_16(co2_sensor->tovc);
	pin = co2_sensor->pin;
	CheckTrigger();
	PushTelemetry();
}

bool DeviceLoraCo2Sensor::CheckData(Json::Value &dataValue, bool& rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember("co2") && dataValue["co2"].isInt())
		{
			int co2 = dataValue["co2"].asInt();
			rs = Util::CompareNumber(this->co2, co2, op);
			return true;
		}
		else if (dataValue.isMember("tovc") && dataValue["tovc"].isInt())
		{
			int tovc = dataValue["tovc"].asInt();
			rs = Util::CompareNumber(this->tovc, tovc, op);
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

bool DeviceLoraCo2Sensor::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}