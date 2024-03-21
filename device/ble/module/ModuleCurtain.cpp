#include "ModuleCurtain.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

enum
{
	CURTAIN_UNKNOWN = -1,
	CURTAIN_CLOSE = 0,
	CURTAIN_OPEN,
	CURTAIN_PAUSE,
	CURTAIN_PERCENT,
};

ModuleCurtain::ModuleCurtain(Device *device, uint16_t addr) : Module(device, addr)
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
	if (dataValue.isObject())
	{
		if (dataValue.isMember(KEY_ATTRIBUTE_CURTAIN) && dataValue[KEY_ATTRIBUTE_CURTAIN].isInt())
		{
			curtain = dataValue[KEY_ATTRIBUTE_CURTAIN].asInt();
			BuildTelemetryValue(jsonValue, dataValue);
			return CODE_OK;
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_MOTOR) && dataValue[KEY_ATTRIBUTE_MOTOR].isInt())
		{
			motor = dataValue[KEY_ATTRIBUTE_MOTOR].asInt();
			BuildTelemetryValue(jsonValue, dataValue);
			return CODE_OK;
		}
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

	uint8_t status;
	Json::Value telemetry;
	telemetry[KEY_ATTRIBUTE_CURTAIN_OPEN] = 0;
	telemetry[KEY_ATTRIBUTE_CURTAIN_CLOSE] = 0;
	telemetry[KEY_ATTRIBUTE_CURTAIN_PAUSE] = 0;
	if (data_message->opcode == 0x52)
	{
		if (data_message->vendorId == RD_OPCODE_PRESS_BUTTON_CURTAN_DOOR_ROOLING || data_message->vendorId == RD_OPCODE_REQUEST_STATUS_CURTAIN)
		{
			status = data_message->header & 0xFF;
			switch (status)
			{
			case CURTAIN_OPEN:
				telemetry[KEY_ATTRIBUTE_CURTAIN_OPEN] = 1;
				break;
			case CURTAIN_CLOSE:
				telemetry[KEY_ATTRIBUTE_CURTAIN_CLOSE] = 1;
				break;
			case CURTAIN_PAUSE:
				if (data_message->vendorId == RD_OPCODE_REQUEST_STATUS_CURTAIN)
				{
					telemetry[KEY_ATTRIBUTE_CURTAIN_OPENED] = (data_message->header >> 8) & 0xFF;
				}
				telemetry[KEY_ATTRIBUTE_CURTAIN_PAUSE] = 1;
				break;
			case CURTAIN_PERCENT:
				telemetry[KEY_ATTRIBUTE_CURTAIN_OPENED] = (data_message->header >> 8) & 0xFF;
				break;
			}
			CheckTrigger();
			BuildTelemetryValue(jsonValue, telemetry);
			return CODE_OK;
		}
	}
	else if (data_message->opcode == RD_OPCODE_CONFIG_RSP)
	{
		if (data_message->header == RD_OPCODE_REQUEST_STATUS_CURTAIN || data_message->header == RD_OPCODE_CONTROL_OPEN_CLOSE_PAUSE)
		{
			status = data_message->type;
			switch (status)
			{
			case CURTAIN_OPEN:
				telemetry[KEY_ATTRIBUTE_CURTAIN_OPEN] = 1;
				break;
			case CURTAIN_CLOSE:
				telemetry[KEY_ATTRIBUTE_CURTAIN_CLOSE] = 1;
				break;
			case CURTAIN_PAUSE:
				if (data_message->vendorId == RD_OPCODE_REQUEST_STATUS_CURTAIN)
				{
					telemetry[KEY_ATTRIBUTE_CURTAIN_OPENED] = (data_message->header >> 8) & 0xFF;
				}
				telemetry[KEY_ATTRIBUTE_CURTAIN_PAUSE] = 1;
				break;
			case CURTAIN_PERCENT:
				telemetry[KEY_ATTRIBUTE_CURTAIN_OPENED] = (data_message->header >> 8) & 0xFF;
				break;
			}
			CheckTrigger();
			BuildTelemetryValue(jsonValue, telemetry);
			return CODE_OK;
		}
	}

	if (data_message->opcode == RD_OPCODE_CONFIG_RSP && data_message->vendorId == RD_VENDOR_ID && data_message->header == RD_OPCODE_CONFIG_MOTOR)
	{
		motor = data_message->type;
		Json::Value dataMotor;
		dataMotor[KEY_ATTRIBUTE_MOTOR] = motor;
		BuildTelemetryValue(jsonValue, dataMotor);
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
		if (dataValue[KEY_ATTRIBUTE_CURTAIN].isArray())
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

void ModuleCurtain::BuildTelemetryValue(Json::Value &jsonValue, Json::Value &telemetryMessage)
{
	jsonValue = telemetryMessage;
}

int ModuleCurtain::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject())
	{
		uint8_t mode = CURTAIN_UNKNOWN;
		int value = -1;
		if (dataValue.isMember(KEY_ATTRIBUTE_CURTAIN_OPEN) && dataValue[KEY_ATTRIBUTE_CURTAIN_OPEN].isInt())
		{
			mode = CURTAIN_OPEN;
			value = dataValue[KEY_ATTRIBUTE_CURTAIN_OPEN].asInt();
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_CURTAIN_CLOSE) && dataValue[KEY_ATTRIBUTE_CURTAIN_CLOSE].isInt())
		{
			mode = CURTAIN_CLOSE;
			value = dataValue[KEY_ATTRIBUTE_CURTAIN_CLOSE].asInt();
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_CURTAIN_PAUSE) && dataValue[KEY_ATTRIBUTE_CURTAIN_PAUSE].isInt())
		{
			mode = CURTAIN_PAUSE;
			value = dataValue[KEY_ATTRIBUTE_CURTAIN_PAUSE].asInt();
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_CURTAIN_OPENED) && dataValue[KEY_ATTRIBUTE_CURTAIN_OPENED].isInt())
		{
			mode = CURTAIN_PERCENT;
			value = dataValue[KEY_ATTRIBUTE_CURTAIN_OPENED].asInt();
		}
		if (mode != -1 && value != -1)
		{
			if (bleProtocol->ControlOpenClosePausePercent(addr, mode, (uint8_t)value) == CODE_OK)
			{
				if (mode == CURTAIN_PERCENT)
				{
					this->curtain = value;
				}
				return CODE_OK;
			}
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_MOTOR) && dataValue[KEY_ATTRIBUTE_MOTOR].isInt())
		{
			int motor = dataValue[KEY_ATTRIBUTE_MOTOR].asInt();
			if (bleProtocol->ConfigMotor(addr, motor) == CODE_OK)
			{
				this->motor = motor;
				return CODE_OK;
			}
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_CALIB_CURTAIN) && dataValue[KEY_ATTRIBUTE_CALIB_CURTAIN].isInt())
		{
			int status = dataValue[KEY_ATTRIBUTE_CALIB_CURTAIN].asInt();
			if (bleProtocol->CalibCurtain(addr, status) == CODE_OK)
			{
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}
