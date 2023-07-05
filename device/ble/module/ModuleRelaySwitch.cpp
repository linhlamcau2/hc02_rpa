#include "ModuleRelaySwitch.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"

ModuleRelaySwitch::ModuleRelaySwitch(Device *device, uint32_t addr, uint8_t relayId) : Module(device, addr)
{
	bt = 0;
	id = BLE_ATTRIBUTE_BUTTON_1 + relayId;
	key = KEY_ATTRIBUTE_BUTTON + to_string(relayId);
}

ModuleRelaySwitch::~ModuleRelaySwitch()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleRelaySwitch::InitAttribute(int id, double bt)
{
	if (this->id == id)
		this->bt = bt;
}

void ModuleRelaySwitch::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, bt);
}
#endif

int ModuleRelaySwitch::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			bt = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}

int ModuleRelaySwitch::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
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
			if (data_message->header == 0x000e || data_message->header == 0x000d || data_message->header == 0x000c || data_message->header == 0x000b)
			{
				if (data_message->relayId == (this->id - 10))
				{
					this->bt = data_message->value;

#ifdef CONFIG_SAVE_ATTRIBUTE
					SaveAttribute();
#endif
					BuildTelemetryValue(jsonValue);
					CheckTrigger();
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
		data_message_t *data_message1 = (data_message_t *)data;
		if (data_message1->header == 0x000e || data_message1->header == 0x000d || data_message1->header == 0x000c || data_message1->header == 0x000b)
		{
			if (data_message1->relayId == (this->id - 10))
			{
				this->bt = data_message1->value;
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

bool ModuleRelaySwitch::CheckData(Json::Value &dataValue, bool &rs)
{
	// LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (dataValue.isObject() &&
			dataValue.isMember("op") && dataValue["op"].isString() &&
			dataValue.isMember(KEY_ATTRIBUTE_BUTTON) && dataValue[KEY_ATTRIBUTE_BUTTON].isInt())
	{
		int value = dataValue[KEY_ATTRIBUTE_BUTTON].asInt();
		string op = dataValue["op"].asString();
		rs = Util::CompareNumber(op, this->bt, value);
		return true;
	}
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
				rs = Util::CompareNumber(op, this->bt, value1, value2);
				return true;
			}
		}
	}
#endif
	return false;
}

void ModuleRelaySwitch::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_BUTTON] = bt;
#else
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = bt;
	jsonValue.append(dataValue);
#endif
}

int ModuleRelaySwitch::Do(Json::Value &dataValue)
{
	LOGD("ModuleRelaySwitch Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_BUTTON) && dataValue[KEY_ATTRIBUTE_BUTTON].isInt())
	{
		int bt = dataValue[KEY_ATTRIBUTE_BUTTON].asInt();
		if (bleProtocol->ControlRelayOfSwitch(addr, device->GetType(), id - 10, bt) == CODE_OK)
		// if (bleProtocol->SetbtLight(addr, bt, 0, true) == CODE_OK)
		{
			this->bt = bt;
			return CODE_OK;
		}
	}
#else
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		LOGW("Id: %d: %d", id, this->id);
		if (this->id == id &&
				dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			if (bleProtocol)
			{
				bleProtocol->ControlRelayOfSwitch(addr, device->GetType(), id - 10, value);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}
