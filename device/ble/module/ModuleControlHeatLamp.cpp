#include "ModuleControlHeatLamp.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

enum
{
	LIGHT_ON = 1,
	COOLING_FAN_ON = 2,
	LIGHT_ON_COOLING_FAN_ON = 3,
	FAN_AC_ON = 4,
	LIGHT_ON_FAN_AC_ON = 5,
	FAN_AC_ON_COOLING_FAN_ON = 6,
	LIGHT_ON_FAN_AC_ON_COOLING_FAN_ON = 7,
	SHOWER2_LIGHT_OFF = 8,
	SHOWER2_LIGHT_ON = 9,
	DRY2_LIGHT_OFF = 22,
	SHOWER1_LIGHT_OFF = 24,
	SHOWER1_LIGHT_ON = 25,
	DRY1_LIGHT_OFF = 32,
	DRY1_LIGHT_ON = 33,
	DRY2_LIGHT_ON = 37,
	OFF_ALL = 128
};

enum
{
	HEATLAMP_MODE_HEATING_1 = 1,
	HEATLAMP_MODE_HEATING_2 = 2,
	HEATLAMP_MODE_DRY = 3,
	HEATLAMP_MODE_OFF = 4
};

ModuleControlHeatLamp::ModuleControlHeatLamp(Device *device, uint32_t addr) : Module(device, addr)
{
	mode = 0;
	light = 0;
	fan = 0;
	coolFan = 0;
	heating = 0;
}

ModuleControlHeatLamp::~ModuleControlHeatLamp()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleControlHeatLamp::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_MODE_HEATLAMP)
		mode = value;
	else if (attribute == KEY_ATTRIBUTE_LIGHT_HEATLAMP)
		light = value;
	else if (attribute == KEY_ATTRIBUTE_FAN_HEATLAMP)
		fan = value;
	else if (attribute == KEY_ATTRIBUTE_COOL_FAN_HEATLAMP)
		coolFan = value;
	else if (attribute == KEY_ATTRIBUTE_HEATING_HEATLAMP)
		heating = value;
}

void ModuleControlHeatLamp::SaveAttribute(string key)
{
	if (key == KEY_ATTRIBUTE_MODE_HEATLAMP)
		database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_MODE_HEATLAMP, mode);
	else if (key == KEY_ATTRIBUTE_LIGHT_HEATLAMP)
		database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_LIGHT_HEATLAMP, light);
	else if (key == KEY_ATTRIBUTE_FAN_HEATLAMP)
		database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_FAN_HEATLAMP, fan);
	else if (key == KEY_ATTRIBUTE_COOL_FAN_HEATLAMP)
		database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_COOL_FAN_HEATLAMP, coolFan);
	else if (key == KEY_ATTRIBUTE_HEATING_HEATLAMP)
		database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_HEATING_HEATLAMP, heating);
}
#endif

int ModuleControlHeatLamp::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject())
	{
		if (dataValue.isMember(KEY_ATTRIBUTE_MODE_HEATLAMP) && dataValue[KEY_ATTRIBUTE_MODE_HEATLAMP].isInt())
		{
			mode = dataValue[KEY_ATTRIBUTE_MODE_HEATLAMP].asInt();
			BuildTelemetryValue(jsonValue, dataValue);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_LIGHT_HEATLAMP) && dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].isInt())
		{
			light = dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].asInt();
			BuildTelemetryValue(jsonValue, dataValue);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_FAN_HEATLAMP) && dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].isInt())
		{
			fan = dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].asInt();
			BuildTelemetryValue(jsonValue, dataValue);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_COOL_FAN_HEATLAMP) && dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].isInt())
		{
			coolFan = dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].asInt();
			BuildTelemetryValue(jsonValue, dataValue);
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_HEATING_HEATLAMP) && dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP].isInt())
		{
			heating = dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP].asInt();
			BuildTelemetryValue(jsonValue, dataValue);
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleControlHeatLamp::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t vendorId;
		uint16_t header;
		uint8_t status;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == RD_OPCODE_CONFIG_RSP &&
		data_message->vendorId == RD_VENDOR_ID)
	{
		Json::Value telemetry;
		if (data_message->header == RD_HEADER_CONTROL_HEAT_LAMP)
		{
			uint8_t value = data_message->status;
			uint8_t templight = 0;
			uint8_t tempfan = 0;
			uint8_t tempcoolFan = 0;
			uint8_t tempheating = 0;
			uint8_t tempmode = 0;

			switch (value)
			{
			case LIGHT_ON:
			{
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 1;
				templight = 1;
				break;
			}
			case COOLING_FAN_ON:
			{
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 1;
				tempcoolFan = 1;
				break;
			}
			case LIGHT_ON_COOLING_FAN_ON:
			{
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 1;
				templight = 1;
				tempcoolFan = 1;
				break;
			}
			case FAN_AC_ON:
			{
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 1;
				tempfan = 1;
				break;
			}
			case LIGHT_ON_FAN_AC_ON:
			{
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 1;
				templight = 1;
				tempfan = 1;
				break;
			}
			case FAN_AC_ON_COOLING_FAN_ON:
			{
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 1;
				tempcoolFan = 1;
				tempfan = 1;
				break;
			}
			case LIGHT_ON_FAN_AC_ON_COOLING_FAN_ON:
			{
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 1;
				templight = 1;
				tempfan = 1;
				tempcoolFan = 1;
				break;
			}
			case SHOWER2_LIGHT_OFF:
			{
				telemetry[KEY_ATTRIBUTE_MODE_HEATLAMP] = 2;
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_HEATING_HEATLAMP] = 1;
				tempmode = 2;
				templight = 0;
				tempfan = 1;
				tempcoolFan = 0;
				tempheating = 1;
				break;
			}
			case SHOWER2_LIGHT_ON:
			{
				telemetry[KEY_ATTRIBUTE_MODE_HEATLAMP] = 2;
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_HEATING_HEATLAMP] = 1;
				tempmode = 2;
				templight = 1;
				tempfan = 1;
				tempcoolFan = 0;
				tempheating = 1;
				break;
			}
			case DRY2_LIGHT_OFF:
			{
				telemetry[KEY_ATTRIBUTE_MODE_HEATLAMP] = 3;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_HEATING_HEATLAMP] = 0;
				tempmode = 3;
				templight = 0;
				tempfan = 0;
				tempcoolFan = 1;
				tempheating = 0;
				break;
			}
			case SHOWER1_LIGHT_OFF:
			{
				telemetry[KEY_ATTRIBUTE_MODE_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_HEATING_HEATLAMP] = 2;
				tempmode = 1;
				templight = 0;
				tempfan = 1;
				tempcoolFan = 0;
				tempheating = 2;
				break;
			}
			case SHOWER1_LIGHT_ON:
			{
				telemetry[KEY_ATTRIBUTE_MODE_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_HEATING_HEATLAMP] = 2;
				tempmode = 1;
				templight = 1;
				tempfan = 1;
				tempcoolFan = 0;
				tempheating = 2;
				break;
			} case DRY1_LIGHT_OFF:
			{
				telemetry[KEY_ATTRIBUTE_MODE_HEATLAMP] = 3;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_HEATING_HEATLAMP] = 2;
				tempmode = 3;
				templight = 0;
				tempfan = 1;
				tempcoolFan = 0;
				tempheating = 2;
				break;
			}
			case DRY1_LIGHT_ON:
			{
				telemetry[KEY_ATTRIBUTE_MODE_HEATLAMP] = 3;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_HEATING_HEATLAMP] = 2;
				tempmode = 3;
				templight = 1;
				tempfan = 1;
				tempcoolFan = 0;
				tempheating = 2;
				break;
			}
			case DRY2_LIGHT_ON:
			{
				telemetry[KEY_ATTRIBUTE_MODE_HEATLAMP] = 3;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 1;
				telemetry[KEY_ATTRIBUTE_HEATING_HEATLAMP] = 0;
				tempmode = 3;
				templight = 1;
				tempfan = 0;
				tempcoolFan = 1;
				tempheating = 0;
				break;
			}
			case OFF_ALL:
			{
				telemetry[KEY_ATTRIBUTE_MODE_HEATLAMP] = 4;
				telemetry[KEY_ATTRIBUTE_FAN_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = 0;
				telemetry[KEY_ATTRIBUTE_HEATING_HEATLAMP] = 0;
				tempmode = 4;
				templight = 0;
				tempfan = 0;
				tempcoolFan = 0;
				tempheating = 0;
				break;
			}
			}
			if (this->mode != tempmode)
			{
				this->mode = tempmode;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_MODE_HEATLAMP); //
#endif
			}
			if (this->light != templight)
			{
				this->light = templight;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_LIGHT_HEATLAMP); //
#endif
			}
			if (this->fan != tempfan)
			{
				this->fan = tempfan;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_FAN_HEATLAMP); //
#endif
			}
			if (this->coolFan != tempcoolFan)
			{
				this->coolFan = tempcoolFan;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_COOL_FAN_HEATLAMP); //
#endif
			}
			if (this->heating != tempheating)
			{
				this->heating = tempheating;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_HEATING_HEATLAMP); //
#endif
			}
			if (this->mode != tempmode)
			{
				this->mode = tempmode;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_MODE_HEATLAMP); //
#endif
			}
			BuildTelemetryValue(jsonValue, telemetry);
			CheckTrigger(telemetry);
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

bool ModuleControlHeatLamp::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_MODE_HEATLAMP) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_MODE_HEATLAMP].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_MODE_HEATLAMP].asInt();
			rs = Util::CompareNumber(op, this->mode, value);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_MODE_HEATLAMP].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_MODE_HEATLAMP];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int value1 = listValue[0].asInt();
				int value2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->mode, value1, value2);
				return true;
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_FAN_HEATLAMP))
		{
			if (dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].isInt())
			{
				int value = dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].asInt();
				rs = Util::CompareNumber(op, this->fan, value);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int value1 = listValue[0].asInt();
					int value2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->fan, value1, value2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_COOL_FAN_HEATLAMP))
		{
			if (dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].isInt())
			{
				int value = dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].asInt();
				rs = Util::CompareNumber(op, this->coolFan, value);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int value1 = listValue[0].asInt();
					int value2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->coolFan, value1, value2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_LIGHT_HEATLAMP))
		{
			if (dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].isInt())
			{
				int value = dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].asInt();
				rs = Util::CompareNumber(op, this->light, value);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int value1 = listValue[0].asInt();
					int value2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->light, value1, value2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_HEATING_HEATLAMP))
		{
			if (dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP].isInt())
			{
				int value = dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP].asInt();
				rs = Util::CompareNumber(op, this->heating, value);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int value1 = listValue[0].asInt();
					int value2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->heating, value1, value2);
					return true;
				}
			}
		}
	}
	return false;
}

void ModuleControlHeatLamp::BuildTelemetryValue(Json::Value &jsonValue, Json::Value &telemetryMessage)
{
	jsonValue = telemetryMessage;
}

int ModuleControlHeatLamp::Do(Json::Value &dataValue)
{
	LOGV("ModuleControlHeatLamp Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject())
	{
		if (dataValue.isMember(KEY_ATTRIBUTE_MODE_HEATLAMP) && dataValue[KEY_ATTRIBUTE_MODE_HEATLAMP].isInt())
		{
			int data = dataValue[KEY_ATTRIBUTE_MODE_HEATLAMP].asInt();
			uint8_t mode = 0;
			uint8_t value = 1;
			switch (data)
			{
			case HEATLAMP_MODE_HEATING_1:
				mode = 8;
				break;
			case HEATLAMP_MODE_HEATING_2:
				mode = 8;
				value = 2;
				break;
			case HEATLAMP_MODE_DRY:
				mode = 16;
				break;
			case HEATLAMP_MODE_OFF:
				mode = 128;
				break;
			}
			if (mode)
			{
				if (bleProtocol->ControlHeatLamp(addr, mode, value) == CODE_OK)
				{
					this->mode = mode;
					return CODE_OK;
				}
			}
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_FAN_HEATLAMP) && dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].isInt())
		{
			uint8_t value = dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].asInt();
			if (bleProtocol->ControlHeatLamp(addr, 2, value) == CODE_OK)
			{
				this->mode = mode;
				return CODE_OK;
			}
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_LIGHT_HEATLAMP) && dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].isInt())
		{
			uint8_t value = dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].asInt();
			if (bleProtocol->ControlHeatLamp(addr, 1, value) == CODE_OK)
			{
				this->mode = mode;
				return CODE_OK;
			}
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_COOL_FAN_HEATLAMP) && dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].isInt())
		{
			uint8_t value = dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].asInt();
			if (bleProtocol->ControlHeatLamp(addr, 4, value) == CODE_OK)
			{
				this->mode = mode;
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}
