#include "ModuleControlPause.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleControlPause::ModuleControlPause(Device *device, uint32_t addr) : Module(device, addr)
{
	value = 1;
	id = BLE_ATTRIBUTE_CURTAIN_PAUSE;
}

ModuleControlPause::~ModuleControlPause()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleControlPause::InitAttribute(int id, double value)
{
	if (this->id == id)
		this->value = value;
}

void ModuleControlPause::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, value);
}
#endif

int ModuleControlPause::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			value = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}

int ModuleControlPause::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t vendorId;
		uint16_t header;
		uint8_t type;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if ((data_message->opcode == 0x52 && data_message->vendorId == RD_OPCODE_PRESS_BUTTON_CURTAN_DOOR_ROOLING && ((data_message->header & 0x00FF) == PAUSE)) || (data_message->opcode == RD_OPCODE_CONFIG_RSP && data_message->header == RD_OPCODE_CONTROL_OPEN_CLOSE_PAUSE && data_message->type == PAUSE))
	{
		value = 1;
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleControlPause::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
				dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
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
				rs = Util::CompareNumber(op, this->value, value1, value2);
				return true;
			}
		}
	}
#endif
	return false;
}

void ModuleControlPause::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_CURTAIN_PAUSE] = value;
#else
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = value;
	jsonValue.append(dataValue);
#endif
}

int ModuleControlPause::Do(Json::Value &dataValue)
{
	LOGD("ModuleControlPause Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_CURTAIN_PAUSE) && dataValue[KEY_ATTRIBUTE_CURTAIN_PAUSE].isInt())
	{
		int value = dataValue[KEY_ATTRIBUTE_CURTAIN_PAUSE].asInt();
		if (value)
		{
			if (bleProtocol->ControlOpenClosePausePercent(addr, PAUSE) == CODE_OK)
			{
				this->value = value;
				return CODE_OK;
			}
		}
	}
#else
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
				dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int percent = dataValue["VALUE"].asInt();
			if (bleProtocol)
			{
				bleProtocol->ControlOpenClosePausePercent(addr, PAUSE);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}
