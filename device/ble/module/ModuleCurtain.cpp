#include "ModuleCurtain.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleCurtain::ModuleCurtain(Device *device, uint32_t addr) : Module(device, addr)
{
	curtain = 0;
}

ModuleCurtain::~ModuleCurtain()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleCurtain::InitAttribute(int id, double value)
{
	if (this->id == id)
		this->curtain = value;
}

void ModuleCurtain::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, curtain);
}
#endif

int ModuleCurtain::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_CURTAIN) && dataValue[KEY_ATTRIBUTE_CURTAIN].isInt())
	{
		curtain = dataValue[KEY_ATTRIBUTE_CURTAIN].asInt();
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleCurtain::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t vendorId;
		uint16_t header;
		uint8_t type;
		uint8_t curtain;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;

	if ((data_message->opcode == 0x52 && (data_message->vendorId == RD_OPCODE_PRESS_BUTTON_CURTAN_DOOR_ROOLING || data_message->vendorId == RD_OPCODE_REQUEST_STATUS_CURTAIN) && ((data_message->header & 0x00FF) == PERCENT)) ||
			(data_message->opcode == RD_OPCODE_CONFIG_RSP && data_message->header == RD_OPCODE_CONTROL_OPEN_CLOSE_PAUSE && data_message->type == PERCENT))
	{
		if (data_message->opcode == 0x52)
		{
			curtain = (data_message->header >> 8) & 0xFF;
		}
		else if (data_message->opcode == RD_OPCODE_CONFIG_RSP)
		{
			curtain = data_message->curtain;
		}
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleCurtain::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_CURTAIN) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_CURTAIN].isInt())
		{
			int curtain = dataValue[KEY_ATTRIBUTE_CURTAIN].asInt();
			rs = Util::CompareNumber(op, this->curtain, curtain);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_CURTAIN].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_CURTAIN];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int curtain1 = listValue[0].asInt();
				int curtain2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->curtain, curtain1, curtain2);
				return true;
			}
		}
	}
	return false;
}

void ModuleCurtain::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_CURTAIN] = curtain;
}

int ModuleCurtain::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_CURTAIN) && dataValue[KEY_ATTRIBUTE_CURTAIN].isInt())
	{
		int curtain = dataValue[KEY_ATTRIBUTE_CURTAIN].asInt();
		if (bleProtocol->ControlOpenClosePausePercent(addr, PERCENT, (uint8_t)curtain) == CODE_OK)
		{
			this->curtain = curtain;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
