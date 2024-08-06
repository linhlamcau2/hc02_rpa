#include "ModuleRelaySwitch.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleRelaySwitch::ModuleRelaySwitch(Device *device, uint16_t addr, uint32_t index) : Module(device, addr, index)
{
	bt = 0;
	key = KEY_ATTRIBUTE_BUTTON + (index ? to_string(index + 1) : "");
}

ModuleRelaySwitch::~ModuleRelaySwitch()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleRelaySwitch::InitAttribute(string attribute, double value)
{
	if (attribute == key)
		this->bt = value;
}

void ModuleRelaySwitch::SaveAttribute()
{
	database->DeviceAttributeAdd(device, key, bt);
}
#endif

int ModuleRelaySwitch::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(key) && dataValue[key].isInt())
	{
		bt = dataValue[key].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleRelaySwitch::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	int temp_bt = 0;
	if (data[0] == RD_OPCODE_CONFIG_RSP)
	{
		typedef struct __attribute__((packed))
		{
			uint8_t opcodeVendor;
			uint16_t vendorId;
			uint16_t header;
			uint8_t relayId;
			uint8_t value;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)data;
		if (data_message->vendorId == RD_VENDOR_ID)
		{
			if (data_message->header == 0x000e ||
				data_message->header == 0x000d ||
				data_message->header == 0x000c ||
				data_message->header == 0x000b)
			{
				if (data_message->relayId == index)
				{
					temp_bt = data_message->value;
					if (temp_bt != bt)
					{
						bt = temp_bt;
#ifdef CONFIG_SAVE_ATTRIBUTE
						SaveAttribute();
#endif
					}
					BuildTelemetryValue(jsonValue);
					CheckTrigger(jsonValue);
					return CODE_OK;
				}
			}
		}
	}
	else if (data[0] == 0x52)
	{
		typedef struct __attribute__((packed))
		{
			uint8_t opcode;
			uint16_t header;
			uint8_t relayId;
			uint8_t value;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)data;
		if (data_message->header == 0x000e || data_message->header == 0x000d || data_message->header == 0x000c || data_message->header == 0x000b)
		{
			if (data_message->relayId == index)
			{
				temp_bt = data_message->value;
				BuildTelemetryValue(jsonValue);
				CheckTrigger(jsonValue);

				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}

bool ModuleRelaySwitch::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(key) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[key].isInt())
		{
			int bt = dataValue[key].asInt();
			rs = Util::CompareNumber(op, this->bt, bt);
			return true;
		}
		else if (dataValue[key].isArray())
		{
			Json::Value listValue = dataValue[key];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int bt1 = listValue[0].asInt();
				int bt2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->bt, bt1, bt2);
				return true;
			}
		}
	}
	return false;
}

void ModuleRelaySwitch::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[key] = bt;
}

int ModuleRelaySwitch::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
		dataValue.isMember(key) && dataValue[key].isInt())
	{
		int bt = dataValue[key].asInt();
		if (bleProtocol->ControlRelayOfSwitch(addr, device->GetType(), index, bt) == CODE_OK)
		{
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
