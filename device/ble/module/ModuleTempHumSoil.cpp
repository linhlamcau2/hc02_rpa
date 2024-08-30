#include "ModuleTempHumSoil.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleTempHumSoil::ModuleTempHumSoil(Device *device, uint16_t addr) : Module(device, addr)
{
	temp = 0;
	hum = 0;
}

ModuleTempHumSoil::~ModuleTempHumSoil()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleTempHumSoil::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_TEMP_SOIL)
		temp = value;
	else if (attribute == KEY_ATTRIBUTE_HUMIDITY_SOIL)
		hum = value;
}

void ModuleTempHumSoil::SaveAttribute(string key)
{
	if (key == KEY_ATTRIBUTE_TEMP_SOIL)
		database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_TEMP_SOIL, temp);
	else if (key == KEY_ATTRIBUTE_HUMIDITY_SOIL)
		database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_HUMIDITY_SOIL, hum);
}
#endif

int ModuleTempHumSoil::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_TEMP_SOIL) && dataValue[KEY_ATTRIBUTE_TEMP_SOIL].isInt() &&
		dataValue.isMember(KEY_ATTRIBUTE_HUMIDITY_SOIL) && dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL].isInt())
	{
		temp = dataValue[KEY_ATTRIBUTE_TEMP_SOIL].asInt();
		hum = dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleTempHumSoil::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint8_t value1[2];
		uint8_t value2[2];
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;

	if (data_message->opcode == 0x52)
	{
		if (data_message->header == RD_HEADER_TEMP_SOIL_STATUS)
		{
			int temp_temp = (((data_message->value1[0] & 0x7F) << 8) | data_message->value1[1]) & 0x7FFF;
			if (data_message->value1[0] & 0x80)
				temp_temp = (-1) * temp_temp;
			int temp_hum = (data_message->value2[0] << 8) | data_message->value2[1];
			if (temp_temp != temp)
			{
				temp = temp_temp;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_TEMP_SOIL);
#endif
			}
			if (temp_hum != hum)
			{
				hum = temp_hum;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_HUMIDITY_SOIL);
#endif
			}
			BuildTelemetryValue(jsonValue);
			CheckTrigger(jsonValue);

			Util::SetTempOfScreenTouch(temp);
			Util::SetHumOfScreenTouch(hum);
			return CODE_OK;
		}
		else if (data_message->header == 0x0107 && len >= 9)
		{
			int temp_hum = (data_message->value1[0] << 8) | data_message->value1[1];
			int temp_temp = (data[7] << 8) | data[8];
			if (data[5] == 0xff)
				temp_temp = (-1) * temp_temp;
			if (temp_temp != temp)
			{
				temp = temp_temp;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_TEMP_SOIL);
#endif
			}
			if (temp_hum != hum)
			{
				hum = temp_hum;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_HUMIDITY_SOIL);
#endif
			}
			BuildTelemetryValue(jsonValue);
			CheckTrigger(jsonValue);

			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

bool ModuleTempHumSoil::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue.isMember(KEY_ATTRIBUTE_TEMP_SOIL))
		{
			if (dataValue[KEY_ATTRIBUTE_TEMP_SOIL].isInt())
			{
				int temp = dataValue[KEY_ATTRIBUTE_TEMP_SOIL].asInt();
				rs = Util::CompareNumber(op, this->temp, temp);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_TEMP_SOIL].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_TEMP_SOIL];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int temp1 = listValue[0].asInt();
					int temp2 = listValue[1].asInt();
					LOGE("temp1 = %d, temp2 = %d, temp = %d", temp1, temp2, this->temp);
					rs = Util::CompareNumber(op, this->temp, temp1, temp2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_HUMIDITY_SOIL))
		{
			if (dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL].isInt())
			{
				int hum = dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL].asInt();
				rs = Util::CompareNumber(op, this->hum, hum);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int hum1 = listValue[0].asInt();
					int hum2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->hum, hum1, hum2);
					return true;
				}
			}
		}
	}
	return false;
}

void ModuleTempHumSoil::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_TEMP_SOIL] = temp;
	jsonValue[KEY_ATTRIBUTE_HUMIDITY_SOIL] = hum;
}
