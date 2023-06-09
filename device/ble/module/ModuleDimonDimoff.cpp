#include "ModuleDimonDimoff.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleDimonDimoff::ModuleDimonDimoff(Device *device, uint32_t addr, uint8_t button) : Module(device, addr)
{
	bt = button;
	dimOn = 0;
	dimOff = 0;
	idDimOn = BLE_ATTRIBUTE_DIM_ON;
	idDimOff = BLE_ATTRIBUTE_DIM_OFF;
	isDimOn = isDimOff = false;
	keyDimOn = KEY_ATTRIBUTE_DIM_ON + to_string(addr - device->GetAddr());
	keyDimOff = KEY_ATTRIBUTE_DIM_OFF + to_string(addr - device->GetAddr());
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
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idDimOff == id || this->idDimOn == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
			{
				if (this->idDimOff == id)
					dimOff = dataValue["VALUE"].asInt();
				else if (this->idDimOn == id)
					dimOn = dataValue["VALUE"].asInt();
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
		}
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
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleDimonDimoff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idDimOn == id || this->idDimOff == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
					dataValue.isMember("OP") && dataValue["OP"].isString())
			{
				uint16_t value1 = 0, value2 = 0;
				string op = dataValue["OP"].asString();
				Json::Value listValue = dataValue["VALUE"];
				if (listValue.size() > 0)
				{
					if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
					{
						value1 = listValue[0].asInt();
						value2 = listValue[1].asInt();
					}
					else if (listValue.size() == 1 && listValue[0].isInt())
					{
						value1 = listValue[0].asInt();
					}
					if (this->idDimOn == id)
						rs = Util::CompareNumber(this->dimOn, value1, value2, op);
					else if (this->idDimOff == id)
						rs = Util::CompareNumber(this->dimOff, value1, value2, op);
					return true;
				}
			}
		}
	}
#endif
	return false;
}

// TODO: can nhac di chuyen den Element.cpp
void ModuleDimonDimoff::CheckTrigger()
{
	LOGV("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

void ModuleDimonDimoff::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[keyDimOn] = dimOn;
	jsonValue[keyDimOff] = dimOff;
#else
	Json::Value dataValue;
	dataValue["ID"] = idDimOn;
	dataValue["VALUE"] = dimOn;
	jsonValue.append(dataValue);
	dataValue["ID"] = idDimOff;
	dataValue["VALUE"] = dimOff;
	jsonValue.append(dataValue);
#endif
}

int ModuleDimonDimoff::Do(Json::Value &dataValue)
{
	LOGD("Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(keyDimOn) && dataValue[keyDimOn].isInt() &&
			dataValue.isMember(keyDimOff) && dataValue[keyDimOff].isInt())
	{
		int dimOn = dataValue[keyDimOn].asInt();
		int dimOff = dataValue[keyDimOff].asInt();
		if (bleProtocol->ControlRgbSwitch(addr, 0, 0, 0, 0, dimOn, dimOff) == CODE_OK)
		{
			this->dimOn = dimOn;
			this->dimOff = dimOff;
			return CODE_OK;
		}
	}
#else
	if (dataValue.isObject())
	{
		if (dataValue.isMember("ID") && dataValue["ID"].isInt() && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			if (dataValue["ID"].asInt() == BLE_ATTRIBUTE_DIM_ON)
			{
				isDimOn = true;
				dimOn = dataValue["VALUE"].asInt();
			}
			else if (dataValue["ID"].asInt() == BLE_ATTRIBUTE_DIM_OFF)
			{
				isDimOff = true;
				dimOff = dataValue["VALUE"].asInt();
			}
		}
		if (isDimOn && isDimOff)
		{
			isDimOff = isDimOn = false;
			if (bleProtocol)
				bleProtocol->ControlRgbSwitch(addr, bt, 0, 0, 0, dimOn, dimOff);
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
#endif
	else
	{
		LOGW("Message format error");
	}
	return CODE_ERROR;
}
