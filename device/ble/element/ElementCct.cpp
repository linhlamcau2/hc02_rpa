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

bool ElementCct::InputData(uint8_t *data, int len, Json::Value &jsonValue, Json::Value &jsonValueV2)
{
	typedef struct
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
			if(cct != data_message->cct_first)
			{
				cct = data_message->cct_first;
				BuildTelemetryValue(jsonValue);
				BuildTelemetryValueV2(jsonValueV2);
			}
		}
		else
		{
			if(cct != data_message->cct)
			{
				cct = data_message->cct;
				BuildTelemetryValue(jsonValue);
				BuildTelemetryValueV2(jsonValueV2);
			}
		}
			cct = data_message->cct;
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return false;
	}
	return true;
}

bool ElementCct::CheckData(Json::Value &dataValue, bool &rs)
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
				rs = Util::CompareNumber(this->cct, cct1, cct2, op);
				return true;
			}
		}
	}
	return false;
}

// TODO: can nhac di chuyen den Element.cpp
void ElementCct::CheckTrigger()
{
	LOGD("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

void ElementCct::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = ((cct - 800) / 192);
	jsonValue.append(dataValue);
}

void ElementCct::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_CCT] = ((cct - 800) / 192);
}

bool ElementCct::Do(Json::Value &dataValue)
{
	LOGD("Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
				dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			uint16_t cct = (value * 192) + 800;
			LOGD("Do cct: %d", cct);
			bleProtocol->SetCctLight(addr, cct, 0, true);
			return true;
		}
	}
	return false;
}

bool ElementCct::DoV2(Json::Value &dataValue)
{
	LOGV("DoV2 data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
	{
		int cct = dataValue[KEY_ATTRIBUTE_CCT].asInt();
		uint16_t value = (cct * 192) + 800;
		if (bleProtocol->SetCctLight(addr, value, 0, true) == CODE_OK)
		{
			this->cct = cct;
		}
		return true;
	}
	return false;
}
