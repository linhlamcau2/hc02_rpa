#include "ElementCct.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementCct::ElementCct(Device *device, uint32_t addr) : Element(device, addr)
{
	cct = 0;
}

ElementCct::~ElementCct()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ElementCct::InitAttribute(int id, double value)
{
	if (this->id == id)
		cct = value;
}

void ElementCct::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, cct);
}
#endif

int ElementCct::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
	{
		cct = dataValue[KEY_ATTRIBUTE_CCT].asInt();
		cct = (cct * 192) + 800;
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ElementCct::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint16_t opcode;
		uint16_t cct_first;
		uint16_t magic;
		uint16_t cct;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == BLE_MESH_OPCODE_CCT)
	{
		if (len <= 6)
		{
			if (cct != data_message->cct_first)
			{
				cct = data_message->cct_first;
				BuildTelemetryValue(jsonValue);
			}
		}
		else
		{
			if (cct != data_message->cct)
			{
				cct = data_message->cct;
				BuildTelemetryValue(jsonValue);
			}
		}
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ElementCct::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_CCT) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_CCT].isInt())
		{
			int cct = dataValue[KEY_ATTRIBUTE_CCT].asInt();
			rs = Util::CompareNumber(op, this->cct, cct);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_CCT].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_CCT];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int cct1 = listValue[0].asInt();
				int cct2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->cct, cct1, cct2);
				return true;
			}
		}
	}
	return false;
}

void ElementCct::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_CCT] = ((cct - 800) / 192);
}

int ElementCct::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
	{
		int cct = dataValue[KEY_ATTRIBUTE_CCT].asInt();
		uint16_t value = (cct * 192) + 800;
		if (bleProtocol->SetCctLight(addr, value, 0, true) == CODE_OK)
		{
			this->cct = cct;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
