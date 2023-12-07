#include "ModuleDimonDimoff.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleDimonDimoff::ModuleDimonDimoff(Device *device, uint16_t addr, uint32_t index) : Module(device, addr, index)
{
	dimOn = 0;
	dimOff = 0;
	keyDimOn = KEY_ATTRIBUTE_DIM_ON + (index ? to_string(index+1) : "");
	keyDimOff = KEY_ATTRIBUTE_DIM_OFF + (index ? to_string(index+1) : "");
}

ModuleDimonDimoff::~ModuleDimonDimoff()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDimonDimoff::InitAttribute(int id, double value)
{
	if (this->id = idDimOn)
	{
		dimOn = value;
	}
	else if (this->id = idDimOff)
	{
		dimOff = value;
	}
}

void ModuleDimonDimoff::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idDimOn, dimOn);
	database->DeviceAttributeAddOrReplace(device, idDimOff, dimOff);
}
#endif

int ModuleDimonDimoff::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(keyDimOn) && dataValue[keyDimOn].isInt() &&
			dataValue.isMember(keyDimOff) && dataValue[keyDimOff].isInt())
	{
		dimOn = dataValue[keyDimOn].asInt();
		dimOff = dataValue[keyDimOff].asInt();
		// CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleDimonDimoff::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t vendorId;
		uint16_t header;
		uint8_t btn;
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t dimOn;
		uint8_t dimOff;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == 0xE3 && data_message->header == 0x050b)
	{
		dimOn = data_message->dimOn;
		dimOff = data_message->dimOff;
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleDimonDimoff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue.isMember(keyDimOn))
		{
			if (dataValue[keyDimOn].isInt())
			{
				int dimOn = dataValue[keyDimOn].asInt();
				rs = Util::CompareNumber(op, this->dimOn, dimOn);
				return true;
			}
			else if (dataValue[keyDimOn].isArray())
			{
				Json::Value listValue = dataValue[keyDimOn];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int dimOn1 = listValue[0].asInt();
					int dimOn2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->dimOn, dimOn1, dimOn2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(keyDimOff))
		{
			if (dataValue[keyDimOff].isInt())
			{
				int dimOff = dataValue[keyDimOff].asInt();
				rs = Util::CompareNumber(op, this->dimOff, dimOff);
				return true;
			}
			else if (dataValue[keyDimOff].isArray())
			{
				Json::Value listValue = dataValue[keyDimOff];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int dimOff1 = listValue[0].asInt();
					int dimOff2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->dimOff, dimOff1, dimOff2);
					return true;
				}
			}
		}
	}
	return false;
}

void ModuleDimonDimoff::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[keyDimOn] = dimOn;
	jsonValue[keyDimOff] = dimOff;
}

int ModuleDimonDimoff::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(keyDimOn) && dataValue[keyDimOn].isInt() &&
			dataValue.isMember(keyDimOff) && dataValue[keyDimOff].isInt())
	{
		int dimOn = dataValue[keyDimOn].asInt();
		int dimOff = dataValue[keyDimOff].asInt();
		if (bleProtocol->ControlRgbSwitch(addr, index, 0, 0, 0, dimOn, dimOff) == CODE_OK)
		{
			this->dimOn = dimOn;
			this->dimOff = dimOff;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
