#include "DeviceLoraECSensor.h"
#include <Log.h>
#include <Util.h>

DeviceLoraECSensor::DeviceLoraECSensor(string id, string name, string mac, uint32_t addr) : DeviceLora(id, name, mac, addr, LORA_SENSOR_EC)
{
	temp = 0;
	mois = 0;
	ec = 0;
	pin = 0;
}

int DeviceLoraECSensor::BuildTelemetryValue(Json::Value &pushDataValue)
{
	pushDataValue["temp"] = temp;
	pushDataValue["mois"] = mois;
	pushDataValue["ec"] = ec;
	pushDataValue["pin"] = pin;
	return 0;
}

void DeviceLoraECSensor::InputData(uint8_t *data, int len, uint32_t addr)
{
	soil_temp_mois_sensor_st *soil_temp_mois_sensor = (soil_temp_mois_sensor_st *)data;
	temp = soil_temp_mois_sensor->temp;
	mois = soil_temp_mois_sensor->mois;
	ec = bswap_16(soil_temp_mois_sensor->ec);
	pin = soil_temp_mois_sensor->pin;
	CheckTrigger();
	PushTelemetry();
}

bool DeviceLoraECSensor::CheckData(Json::Value &dataValue, bool& rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember("mois") && dataValue["mois"].isInt())
		{
			int mois = dataValue["mois"].asInt();
			rs = Util::CompareNumber(this->mois, mois, op);
			return true;
		}
		else if (dataValue.isMember("temp") && dataValue["temp"].isInt())
		{
			int temp = dataValue["temp"].asInt();
			rs = Util::CompareNumber(this->temp, temp, op);
			return true;
		}
		else if (dataValue.isMember("ec") && dataValue["ec"].isInt())
		{
			int ec = dataValue["ec"].asInt();
			rs = Util::CompareNumber(this->ec, ec, op);
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

bool DeviceLoraECSensor::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}
