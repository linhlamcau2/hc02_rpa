#include "DeviceLoraTempHumSensor.h"
#include <Log.h>
#include <Util.h>

DeviceLoraTempHumSensor::DeviceLoraTempHumSensor(string id, string name, string mac, uint32_t addr) : DeviceLora(id, name, mac, addr, LORA_SENSOR_HUM_TEMP)
{
	temp = 0;
	hum = 0;
	pin = 0;
}

int DeviceLoraTempHumSensor::BuildTelemetryValue(Json::Value &pushDataValue)
{
	pushDataValue["temp"] = temp;
	pushDataValue["hum"] = hum;
	pushDataValue["pin"] = pin;
	return 0;
}

void DeviceLoraTempHumSensor::InputData(uint8_t *data, int len, uint32_t addr)
{
	temp_hum_sensor_st *temp_hum_sensor = (temp_hum_sensor_st *)data;
	temp = bswap_16(temp_hum_sensor->temp)/100;
	hum = bswap_16(temp_hum_sensor->hum)/100;
	pin = temp_hum_sensor->pin;
	CheckTrigger();
	PushTelemetry();
}

bool DeviceLoraTempHumSensor::CheckData(Json::Value &dataValue, bool& rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember("temp") && dataValue["temp"].isInt())
		{
			int temp = dataValue["temp"].asInt();
			rs = Util::CompareNumber(this->temp, temp, op);
			return true;
		}
		else if (dataValue.isMember("hum") && dataValue["hum"].isInt())
		{
			int hum = dataValue["hum"].asInt();
			rs = Util::CompareNumber(this->hum, hum, op);
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

bool DeviceLoraTempHumSensor::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}
