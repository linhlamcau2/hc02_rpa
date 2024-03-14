#include "ModuleModeRgb.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleModeRgb::ModuleModeRgb(Device *device, uint16_t addr) : Module(device, addr)
{
	mode = 0;
}

ModuleModeRgb::~ModuleModeRgb()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleModeRgb::InitAttribute(string attridute, double value)
{
	if (attridute == KEY_ATTRIBUTE_MODE_RGB)
		mode = value;
}

void ModuleModeRgb::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, KEY_ATTRIBUTE_MODE_RGB, mode);
}
#endif

int ModuleModeRgb::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && dataValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
	{
		mode = dataValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
		// CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleModeRgb::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint16_t opcode;
		uint16_t reverse;
		uint16_t idScene;
		uint16_t magic;
		uint8_t mode;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == BLE_MESH_OPCODE_RGB)
	{
		if (data_message->idScene == 0)
		{
			mode = data_message->mode;
			// TODO: recheck
			if (1 <= mode && mode <= 6)
			{
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute();
#endif
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}

bool ModuleModeRgb::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_MODE_RGB) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
		{
			int mode = dataValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
			rs = Util::CompareNumber(op, this->mode, mode);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_MODE_RGB].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_MODE_RGB];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int mode1 = listValue[0].asInt();
				int mode2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->mode, mode1, mode2);
				return true;
			}
		}
	}
	return false;
}

void ModuleModeRgb::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_MODE_RGB] = mode;
}

int ModuleModeRgb::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && dataValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
	{
		int mode = dataValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
		if (bleProtocol->CallModeRgb(addr, mode) == CODE_OK)
		{
			this->mode = mode;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
