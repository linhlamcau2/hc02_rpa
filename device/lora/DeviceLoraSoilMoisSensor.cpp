#include "DeviceLoraSoilMoisSensor.h"
#include <Log.h>
#include <Util.h>

DeviceLoraSoilMoisSensor::DeviceLoraSoilMoisSensor(string id, string name, string mac, uint32_t addr) : DeviceLora(id, name, mac, addr, LORA_SENSOR_SOIL_MOISTURE)
{
	mois = 0;
	pin = 0;
}

int DeviceLoraSoilMoisSensor::BuildTelemetryValue(Json::Value &pushDataValue)
{
	pushDataValue["mois"] = mois;
	pushDataValue["pin"] = pin;
	return 0;
}

void DeviceLoraSoilMoisSensor::InputData(uint8_t *data, int len, uint32_t addr)
{
	soil_mois_sensor_st *soil_mois_sensor = (soil_mois_sensor_st *)data;
	mois = bswap_32(soil_mois_sensor->mois);
	pin = soil_mois_sensor->pin;
	CheckTrigger();
	PushTelemetry();
}

bool DeviceLoraSoilMoisSensor::CheckData(Json::Value &dataValue, bool& rs)
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
		else if (dataValue.isMember("pin") && dataValue["pin"].isInt())
		{
			int pin = dataValue["pin"].asInt();
			rs = Util::CompareNumber(this->pin, pin, op);
			return true;
		}
	}
	return false;
}

bool DeviceLoraSoilMoisSensor::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}
