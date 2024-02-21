#include "ModuleModeRgb.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleModeRgb::ModuleModeRgb(Device *device, uint32_t addr) : Module(device, addr)
{
	mode = 0;
	id = BLE_ATTRIBUTE_SCENE_RGB;
}

ModuleModeRgb::~ModuleModeRgb()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleModeRgb::InitAttribute(int id, double value)
{
	if (this->id == id)
		mode = value;
}

void ModuleModeRgb::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, mode);
}
#endif

int ModuleModeRgb::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isArray())
	{
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			if (dataValue[i].isObject() && dataValue[i].isMember("ID") && dataValue[i]["ID"].isInt())
			{
				int id = dataValue[i]["ID"].asInt();
				if (this->id == id && dataValue[i].isMember("VALUE") && dataValue[i]["VALUE"].isInt())
				{
					mode = dataValue[i]["VALUE"].asInt();
					BuildTelemetryValue(jsonValue);
					return CODE_OK;
				}
			}
		}
	}
	else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
			{
				mode = dataValue["VALUE"].asInt();
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
		}
	}
#endif
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
			uint16_t mode1 = 0, mode2 = 0;
			string op = dataValue["OP"].asString();
			Json::Value listValue = dataValue["VALUE"];
			if (listValue.size() > 0)
			{
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					mode1 = listValue[0].asInt();
					mode2 = listValue[1].asInt();
				}
				else if (listValue.size() == 1 && listValue[0].isInt())
				{
					mode1 = listValue[0].asInt();
				}
				rs = Util::CompareNumber(op, this->mode, mode1, mode2);
				return true;
			}
		}
	}
#endif
	return false;
}

void ModuleModeRgb::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_MODE_RGB] = mode;
#else
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = mode;
	jsonValue.append(dataValue);
#endif
}

int ModuleModeRgb::Do(Json::Value &dataValue)
{
	LOGD("ModuleModeRgb Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
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
#else
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
				dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			if (bleProtocol)
			{
				bleProtocol->CallModeRgb(addr, value);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}
