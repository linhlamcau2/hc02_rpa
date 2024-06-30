#include "ModuleLightSensor.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleLightSensor::ModuleLightSensor(Device *device, uint16_t addr) : Module(device, addr)
{
	lux = 0;
}

ModuleLightSensor::~ModuleLightSensor()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleLightSensor::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_LUX)
		lux = value;
}

void ModuleLightSensor::SaveAttribute()
{
	database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_LUX, lux);
}
#endif

int ModuleLightSensor::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_LUX) && dataValue[KEY_ATTRIBUTE_LUX].isInt())
	{
		lux = dataValue[KEY_ATTRIBUTE_LUX].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

static uint16_t CalculateLux(uint16_t rsp_lux)
{
	unsigned int lux_LSB = 0;
	unsigned char lux_MSB = 0;
	uint16_t lux_Value = 0;
	unsigned int pow = 1;
	unsigned char i;
	lux_LSB = rsp_lux & 0x0FFF;
	lux_MSB = ((rsp_lux >> 12) & 0x0F);
	// Lux_Value = 0.01 * pow(2,Lux_MSB) * Lux_LSB; //don't use
	for (i = 0; i < lux_MSB; i++)
	{
		pow = pow * 2;
	}
	lux_Value = 0.01 * pow * lux_LSB;
	return lux_Value;
}

int ModuleLightSensor::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52)
	{
		if (data[1] == 0x04 && (data[2] == 0x00 || data[2] == 0x01))
		{
			typedef struct __attribute__((packed))
			{
				uint16_t lux;
				uint16_t scene;
			} data_message_t;
			data_message_t *data_message = (data_message_t *)&data[3];
			uint16_t temp_lux = 0;
			if (data[2] == 0x00)
			{
				temp_lux = CalculateLux(bswap_16(data_message->lux));
			}
			else if (data[2] == 0x01)
			{
				temp_lux = data_message->lux;
			}

			if (temp_lux != lux)
			{
				lux = temp_lux;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute();
#endif
			}
			BuildTelemetryValue(jsonValue);
			CheckTrigger(jsonValue);

			return false;
		}
	}
	return true;
}

bool ModuleLightSensor::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_LUX) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_LUX].isInt())
		{
			int lux = dataValue[KEY_ATTRIBUTE_LUX].asInt();
			rs = Util::CompareNumber(op, this->lux, lux);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_LUX].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_LUX];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int lux1 = listValue[0].asInt();
				int lux2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->lux, lux1, lux2);
				return true;
			}
		}
	}
	return false;
}

void ModuleLightSensor::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_LUX] = lux;
}
