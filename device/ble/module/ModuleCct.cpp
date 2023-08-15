#include "ModuleCct.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleCct::ModuleCct(Device *device, uint16_t addr) : Module(device, addr)
{
	cct = 0;
}

ModuleCct::~ModuleCct()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleCct::InitAttribute(int id, double value)
{
	if (this->id == id)
		cct = value;
}

void ModuleCct::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, cct);
}
#endif

int ModuleCct::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
	{
		cct = dataValue[KEY_ATTRIBUTE_CCT].asInt();
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleCct::InputData(uint8_t *data, int len, Json::Value &jsonValue)
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
			cct = (data_message->cct_first - 800) / 192;
		}
		else
		{
			cct = (data_message->cct - 800) / 192;
		}
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleCct::CheckData(Json::Value &dataValue, bool &rs)
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

void ModuleCct::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_CCT] = cct;
}

int ModuleCct::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
	{
		int cct = dataValue[KEY_ATTRIBUTE_CCT].asInt();
		if (bleProtocol->SetCctLight(addr, (cct * 192) + 800, 0, true) == CODE_OK)
		{
			this->cct = cct;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
