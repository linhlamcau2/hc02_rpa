#include "ModuleRgb.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleRgb::ModuleRgb(Device *device, uint16_t addr, uint32_t index) : Module(device, addr, index)
{
	r = 0;
	b = 0;
	g = 0;
	dimOn = 0;
	dimOff = 0;
	keyR = KEY_ATTRIBUTE_R + (index ? to_string(index + 1) : "");
	keyG = KEY_ATTRIBUTE_G + (index ? to_string(index + 1) : "");
	keyB = KEY_ATTRIBUTE_B + (index ? to_string(index + 1) : "");
	keyDimOn = KEY_ATTRIBUTE_DIM_ON + (index ? to_string(index + 1) : "");
	keyDimOff = KEY_ATTRIBUTE_DIM_OFF + (index ? to_string(index + 1) : "");
}

ModuleRgb::~ModuleRgb()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleRgb::InitAttribute(int id, double value)
{
	if (this->id == idR)
	{
		r = value;
	}
	else if (this->id = idG)
	{
		g = value;
	}
	else if (this->id = idB)
	{
		b = value;
	}
	else if (this->id = idDimOn)
	{
		dimOn = value;
	}
	else if (this->id = idDimOff)
	{
		dimOff = value;
	}
}

void ModuleRgb::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idR, r);
	database->DeviceAttributeAddOrReplace(device, idG, g);
	database->DeviceAttributeAddOrReplace(device, idB, b);
	database->DeviceAttributeAddOrReplace(device, idDimOn, dimOn);
	database->DeviceAttributeAddOrReplace(device, idDimOff, dimOff);
}
#endif

int ModuleRgb::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(keyR) && dataValue[keyR].isInt() &&
		dataValue.isMember(keyG) && dataValue[keyG].isInt() &&
		dataValue.isMember(keyB) && dataValue[keyB].isInt() &&
		dataValue.isMember(keyDimOn) && dataValue[keyDimOn].isInt() &&
		dataValue.isMember(keyDimOff) && dataValue[keyDimOff].isInt())
	{
		r = dataValue[keyR].asInt();
		g = dataValue[keyG].asInt();
		b = dataValue[keyB].asInt();
		dimOn = dataValue[keyDimOn].asInt();
		dimOff = dataValue[keyDimOff].asInt();
		// CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleRgb::InputData(uint8_t *data, int len, Json::Value &jsonValue)
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
		b = data_message->b;
		g = data_message->g;
		r = data_message->r;
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

bool ModuleRgb::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue.isMember(keyR))
		{
			if (dataValue[keyR].isInt())
			{
				int r = dataValue[keyR].asInt();
				rs = Util::CompareNumber(op, this->r, r);
				return true;
			}
			else if (dataValue[keyR].isArray())
			{
				Json::Value listValue = dataValue[keyR];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int r1 = listValue[0].asInt();
					int r2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->r, r1, r2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(keyG))
		{
			if (dataValue[keyG].isInt())
			{
				int g = dataValue[keyG].asInt();
				rs = Util::CompareNumber(op, this->g, g);
				return true;
			}
			else if (dataValue[keyG].isArray())
			{
				Json::Value listValue = dataValue[keyG];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int g1 = listValue[0].asInt();
					int g2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->g, g1, g1);
					return true;
				}
			}
		}
		else if (dataValue.isMember(keyB))
		{
			if (dataValue[keyB].isInt())
			{
				int b = dataValue[keyB].asInt();
				rs = Util::CompareNumber(op, this->b, b);
				return true;
			}
			else if (dataValue[keyB].isArray())
			{
				Json::Value listValue = dataValue[keyB];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int b1 = listValue[0].asInt();
					int b2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->b, b1, b2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(keyDimOn))
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

void ModuleRgb::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[keyR] = r;
	jsonValue[keyG] = g;
	jsonValue[keyB] = b;
	jsonValue[keyDimOn] = dimOn;
	jsonValue[keyDimOff] = dimOff;
}

int ModuleRgb::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
		dataValue.isMember(keyR) && dataValue[keyR].isInt() &&
		dataValue.isMember(keyG) && dataValue[keyG].isInt() &&
		dataValue.isMember(keyB) && dataValue[keyB].isInt() &&
		dataValue.isMember(keyDimOn) && dataValue[keyDimOn].isInt() &&
		dataValue.isMember(keyDimOff) && dataValue[keyDimOff].isInt())
	{
		int r = dataValue[keyR].asInt();
		int g = dataValue[keyG].asInt();
		int b = dataValue[keyB].asInt();
		int dimOn = dataValue[keyDimOn].asInt();
		int dimOff = dataValue[keyDimOff].asInt();
		if (bleProtocol->ControlRgbSwitch(addr, index + 1, b, g, r, dimOn, dimOff) == CODE_OK)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->dimOn = dimOn;
			this->dimOff = dimOff;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
