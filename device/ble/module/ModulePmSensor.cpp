#include "ModulePmSensor.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModulePmSensor::ModulePmSensor(Device *device, uint16_t addr) : Module(device, addr)
{
	pm25 = 0;
	pm10 = 0;
	pm1_0 = 0;
}

ModulePmSensor::~ModulePmSensor()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModulePmSensor::InitAttribute(int id, double value)
{
	if (this->id == idPm25)
		pm25 = value;
	else if (this->id == idPm10)
		pm10 = value;
	else if (this->id == idPm1_0)
		pm1_0 = value;
}

void ModulePmSensor::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idPm25, pm25);
	database->DeviceAttributeAddOrReplace(device, idPm10, pm10);
	database->DeviceAttributeAddOrReplace(device, idPm1_0, pm1_0);
}
#endif

int ModulePmSensor::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_PM2_5) && dataValue[KEY_ATTRIBUTE_PM2_5].isInt() &&
			dataValue.isMember(KEY_ATTRIBUTE_PM10) && dataValue[KEY_ATTRIBUTE_PM10].isInt() &&
			dataValue.isMember(KEY_ATTRIBUTE_PM1_0) && dataValue[KEY_ATTRIBUTE_PM1_0].isInt())
	{
		pm1_0 = dataValue[KEY_ATTRIBUTE_PM2_5].asInt();
		pm10 = dataValue[KEY_ATTRIBUTE_PM10].asInt();
		pm1_0 = dataValue[KEY_ATTRIBUTE_PM1_0].asInt();
		// CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModulePmSensor::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x07 && data[2] == 0x02)
	{
		typedef struct __attribute__((packed))
		{
			uint16_t pm25;
			uint16_t pm10;
			uint16_t pm1_0;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)&data[3];
		pm25 = bswap_16(data_message->pm25);
		pm10 = bswap_16(data_message->pm10);
		pm1_0 = bswap_16(data_message->pm1_0);
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModulePmSensor::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue.isMember(KEY_ATTRIBUTE_PM2_5))
		{
			if (dataValue[KEY_ATTRIBUTE_PM2_5].isInt())
			{
				int pm1_0 = dataValue[KEY_ATTRIBUTE_PM2_5].asInt();
				rs = Util::CompareNumber(op, this->pm1_0, pm1_0);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_PM2_5].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_PM2_5];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int pm1_01 = listValue[0].asInt();
					int pm1_02 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->pm1_0, pm1_01, pm1_02);
					return true;
				}
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_PM10))
		{
			if (dataValue[KEY_ATTRIBUTE_PM10].isInt())
			{
				int pm10 = dataValue[KEY_ATTRIBUTE_PM10].asInt();
				rs = Util::CompareNumber(op, this->pm10, pm10);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_PM10].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_PM10];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int pm101 = listValue[0].asInt();
					int pm102 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->pm10, pm101, pm102);
					return true;
				}
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_PM1_0))
		{
			if (dataValue[KEY_ATTRIBUTE_PM1_0].isInt())
			{
				int pm1_0 = dataValue[KEY_ATTRIBUTE_PM1_0].asInt();
				rs = Util::CompareNumber(op, this->pm1_0, pm1_0);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_PM1_0].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_PM1_0];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int pm1_01 = listValue[0].asInt();
					int pm1_02 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->pm1_0, pm1_01, pm1_02);
					return true;
				}
			}
		}
	}
	return false;
}

void ModulePmSensor::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_PM2_5] = pm25;
	jsonValue[KEY_ATTRIBUTE_PM10] = pm10;
	jsonValue[KEY_ATTRIBUTE_PM1_0] = pm1_0;
}
