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
	id = BLE_ATTRIBUTE_CCT;
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
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			cct = (dataValue["VALUE"].asInt() * 192) + 800;

			BuildTelemetryValue(jsonValue);
			// CheckTrigger();
			return CODE_OK;
		}
	}
#endif
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
	// LOGD("CheckData data: %s", dataValue.toString().c_str());
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
			uint16_t cct1 = 0, cct2 = 0;
			string op = dataValue["OP"].asString();
			Json::Value listValue = dataValue["VALUE"];
			if (listValue.size() > 0)
			{
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					cct1 = listValue[0].asInt();
					cct2 = listValue[1].asInt();
				}
				else if (listValue.size() == 1 && listValue[0].isInt())
				{
					cct1 = listValue[0].asInt();
				}
				rs = Util::CompareNumber(op, this->cct, cct1, cct2);
				return true;
			}
		}
	}
#endif
	return false;
}

void ElementCct::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_CCT] = ((cct - 800) / 192);
#else
	int valueCctPush = ((cct - 800) / 192);
	if (valueCctPush >= 0 && valueCctPush <= 100)
	{
		Json::Value dataValue;
		dataValue["ID"] = id;
		dataValue["VALUE"] = valueCctPush;
		jsonValue.append(dataValue);
	}

#endif
}

int ElementCct::Do(Json::Value &dataValue)
{
	// LOGD("Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (bleProtocol && dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
	{
		int cct = dataValue[KEY_ATTRIBUTE_CCT].asInt();
		uint16_t value = (cct * 192) + 800;
		if (bleProtocol->SetCctLight(addr, value, TRANSITION_DEFAULT, true) == CODE_OK)
		{
			this->cct = cct;
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
			uint16_t cct = (value * 192) + 800;
			if (bleProtocol)
				bleProtocol->SetCctLight(addr, cct, TRANSITION_DEFAULT, true);
			else
				LOGW("Bleprotocol null");
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}
