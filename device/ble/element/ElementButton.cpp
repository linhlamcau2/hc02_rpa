#include "ElementButton.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementButton::ElementButton(Device *device, uint32_t addr) : Element(device, addr)
{
	bt = 0;
	key = KEY_ATTRIBUTE_BUTTON + to_string(addr - device->GetAddr());
}

ElementButton::~ElementButton()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ElementButton::InitAttribute(int id, double value)
{
	if (this->id == id)
		bt = value;
}

void ElementButton::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, bt);
}
#endif

int ElementButton::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(key) && dataValue[key].isInt())
	{
		bt = dataValue[key].asInt();
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ElementButton::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint16_t opcode;
		uint8_t state;
		uint8_t bt;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == BLE_MESH_OPCODE_ONOFF)
	{
		if (len == 3)
			bt = data_message->state;
		else
			bt = data_message->bt;
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ElementButton::CheckData(Json::Value &dataValue, bool &rs)
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

void ElementButton::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[key] = bt;
}

int ElementButton::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(key) && dataValue[key].isInt())
	{
		int bt = dataValue[key].asInt();
		if (bleProtocol->SetOnOffLight(addr, bt, 0, true) == CODE_OK)
		{
			this->bt = bt;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}