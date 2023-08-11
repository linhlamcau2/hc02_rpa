#include "ModuleSelectMotor.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleSelectMotor::ModuleSelectMotor(Device *device, uint32_t addr) : Module(device, addr)
{
	motor = 0;
}

ModuleSelectMotor::~ModuleSelectMotor()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleSelectMotor::InitAttribute(int id, double value)
{
}

void ModuleSelectMotor::SaveAttribute()
{
}
#endif

int ModuleSelectMotor::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_MOTOR) && dataValue[KEY_ATTRIBUTE_MOTOR].isInt())
	{
		motor = dataValue[KEY_ATTRIBUTE_MOTOR].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleSelectMotor::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t vendorId;
		uint16_t header;
		uint8_t type;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == RD_OPCODE_CONFIG_RSP && data_message->vendorId == RD_VENDOR_ID && data_message->header == RD_OPCODE_CONFIG_MOTOR)
	{
		motor = data_message->type;
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

void ModuleSelectMotor::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_MOTOR] = motor;
}

int ModuleSelectMotor::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_MOTOR) && dataValue[KEY_ATTRIBUTE_MOTOR].isInt())
	{
		int motor = dataValue[KEY_ATTRIBUTE_MOTOR].asInt();
		if (bleProtocol->ConfigMotor(addr, motor) == CODE_OK)
		{
			this->motor = motor;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
