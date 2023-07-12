#include "ModuleOnOff.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleOnOff::ModuleOnOff(Device *device, uint32_t addr) : Module(device, addr)
{
	onoff = 0;
}

ModuleOnOff::~ModuleOnOff()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleOnOff::InitAttribute(int id, double value)
{
	if (this->id == id)
		onoff = value;
}

void ModuleOnOff::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, onoff);
}
#endif

int ModuleOnOff::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_ONOFF) && dataValue[KEY_ATTRIBUTE_ONOFF].isInt())
	{
		uint8_t onoff = dataValue[KEY_ATTRIBUTE_ONOFF].asInt();
		if (this->onoff != onoff)
		{
			this->onoff = onoff;
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleOnOff::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint16_t opcode;
		uint8_t state;
		uint8_t onoff;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == BLE_MESH_OPCODE_ONOFF)
	{
		uint8_t onoff = 0;
		if (len == 3)
		{
			onoff = data_message->state;
		}
		else
		{
			onoff = data_message->onoff;
		}
		if (this->onoff != onoff)
		{
			this->onoff = onoff;
#ifdef CONFIG_SAVE_ATTRIBUTE
			SaveAttribute();
#endif
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleOnOff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_ONOFF) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_ONOFF].isInt())
		{
			int onoff = dataValue[KEY_ATTRIBUTE_ONOFF].asInt();
			rs = Util::CompareNumber(op, this->onoff, onoff);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_ONOFF].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_ONOFF];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int onoff1 = listValue[0].asInt();
				int onoff2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->onoff, onoff1, onoff2);
				return true;
			}
		}
	}
	return false;
}

void ModuleOnOff::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_ONOFF] = onoff;
}

int ModuleOnOff::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_ONOFF) && dataValue[KEY_ATTRIBUTE_ONOFF].isInt())
	{
		int onoff = dataValue[KEY_ATTRIBUTE_ONOFF].asInt();
		if (bleProtocol->SetOnOffLight(addr, onoff, 0, true) == CODE_OK)
		{
			this->onoff = onoff;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
