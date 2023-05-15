#include "ModuleControlPercent.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleControlPercent::ModuleControlPercent(Device *device, uint32_t addr) : Module(device, addr)
{
	percent = 1;
	id = BLE_ATTRIBUTE_CURTAIN_OPENED;
}

ModuleControlPercent::~ModuleControlPercent()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleControlPercent::InitAttribute(int id, double value)
{
	if (this->id == id)
		this->percent = value;
}

void ModuleControlPercent::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, percent);
}
#endif

int ModuleControlPercent::InputData(Json::Value &dataValue, Json::Value &jsonValue, Json::Value &jsonValueV2)
{
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			percent = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

int ModuleControlPercent::InputData(uint8_t *data, int len, Json::Value &jsonValue, Json::Value &jsonValueV2)
{
	typedef struct
	{
		uint8_t opcode;
		uint16_t vendorId;
		uint16_t header;
		uint8_t type;
		uint8_t percent;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;

	if ((data_message->opcode == 0x52 && (data_message->vendorId == RD_OPCODE_PRESS_BUTTON_CURTAN_DOOR_ROOLING || data_message->vendorId == RD_OPCODE_REQUEST_STATUS_CURTAIN) && ((data_message->header & 0x00FF) == PERCENT)) ||
		(data_message->opcode == RD_OPCODE_CONFIG_RSP && data_message->header == RD_OPCODE_CONTROL_OPEN_CLOSE_PAUSE && data_message->type == PERCENT))
	{
		if (data_message->opcode == 0x52)
		{
			percent = (data_message->header >> 8) & 0xFF;
		}
		else if (data_message->opcode == RD_OPCODE_CONFIG_RSP)
		{
			percent = data_message->percent;
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

bool ModuleControlPercent::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
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
					value2 = listValue[0].asInt();
				}
				rs = Util::CompareNumber(this->percent, value1, value2, op);
				return true;
			}
		}
	}
	return false;
}

void ModuleControlPercent::CheckTrigger()
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

void ModuleControlPercent::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = percent;
	jsonValue.append(dataValue);
}

int ModuleControlPercent::Do(Json::Value &dataValue)
{
	LOGD("ModuleControl Percent Do data: %s", dataValue.toString().c_str());
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
				bleProtocol->ControlOpenClosePausePercent(addr, PERCENT, (uint8_t)percent);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
void ModuleControlPercent::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_CURTAIN_OPENED] = percent;
}

int ModuleControlPercent::DoV2(Json::Value &dataValue)
{
	LOGV("DoV2 data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_CURTAIN_OPENED) && dataValue[KEY_ATTRIBUTE_CURTAIN_OPENED].isInt())
	{
		int percent = dataValue[KEY_ATTRIBUTE_CURTAIN_OPENED].asInt();
		if (bleProtocol->ControlOpenClosePausePercent(addr, PERCENT, (uint8_t)percent) == CODE_OK)
		{
			this->percent = percent;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
#endif // CONFIG_USE_MESSAGE_FORMAT_V2
