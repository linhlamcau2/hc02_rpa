#include "ModuleDim.h"
#include <Log.h>
#include <Util.h>
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleDim::ModuleDim(Device *device, uint32_t addr) : Module(device, addr)
{
	dim = 0;
	id = BLE_ATTRIBUTE_DIM;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDim::InitAttribute(int id, double value)
{
	if (this->id == id)
		dim = value;
}

void ModuleDim::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, dim);
}
#endif

bool ModuleDim::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct
	{
		uint16_t opcode;
		uint16_t dim_first;
		uint16_t dim;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == BLE_MESH_OPCODE_DIM)
	{
		if (len <= 5)
			dim = data_message->dim_first;
		else
			dim = data_message->dim;
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return true;
	}
	return false;
}

bool ModuleDim::CheckData(Json::Value &dataValue, bool &rs)
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
			uint16_t dim1, dim2;
			Json::Value listValue = dataValue["VALUE"];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				dim1 = listValue[0].asInt();
				dim2 = listValue[1].asInt();
			}
			string op = dataValue["OP"].asString();
			rs = Util::CompareNumber(this->dim, dim1, dim2, op);
			return true;
		}
	}
	return false;
}

void ModuleDim::CheckTrigger()
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

void ModuleDim::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = dim;
	jsonValue.append(dataValue);
}

bool ModuleDim::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
				dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			uint16_t dim = (value * 65535) / 100;
			bleProtocol->SetDimmingLight(addr, dim, 0, true);
			return true;
		}
	}
	return false;
}
