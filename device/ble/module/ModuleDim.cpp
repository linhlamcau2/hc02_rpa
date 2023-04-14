#include "ModuleDim.h"
#include "Log.h"
#include "Util.h"
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

int ModuleDim::InputData(uint8_t *data, int len, Json::Value &jsonValue, Json::Value &jsonValueV2)
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
		{
			if(dim != data_message->dim_first)
			{
				dim = data_message->dim_first;
				BuildTelemetryValue(jsonValue);
				BuildTelemetryValueV2(jsonValueV2);
			}
		}
		else
		{
			if(dim != data_message->dim)
			{
				dim = data_message->dim;
				BuildTelemetryValue(jsonValue);
				BuildTelemetryValueV2(jsonValueV2);
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
			uint16_t dim1 = 0, dim2 = 0;
			string op = dataValue["OP"].asString();
			Json::Value listValue = dataValue["VALUE"];
			if (listValue.size() > 0)
			{
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					dim1 = listValue[0].asInt();
					dim2 = listValue[1].asInt();
				}
				else if (listValue.size() == 1 && listValue[0].isInt())
				{
					dim1 = listValue[0].asInt();
				}
				rs = Util::CompareNumber(this->dim, dim1, dim2, op);
				return true;
			}
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
	dataValue["VALUE"] = (dim * 100) / 65535;
	jsonValue.append(dataValue);
}

void ModuleDim::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_DIM] = dim*100/65535;
}

int ModuleDim::Do(Json::Value &dataValue)
{
	LOGD("ModuleDim Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
			dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			uint16_t dim = (value * 65535) / 100;
			if (bleProtocol)
			{
				bleProtocol->SetDimmingLight(addr, dim, 0, true);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

int ModuleDim::DoV2(Json::Value &dataValue)
{
	LOGV("DoV2 data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_DIM) && dataValue[KEY_ATTRIBUTE_DIM].isInt())
	{
		int dim = dataValue[KEY_ATTRIBUTE_DIM].asInt();
		uint16_t value = (dim * 65535) / 100;
		if (bleProtocol)
		{
			if (bleProtocol->SetDimmingLight(addr, value, 0, true) == CODE_OK)
			{
				this->dim = dim;
			}
		}
		else
			LOGW("BleProtocol null");
		return CODE_OK;
	}
	return CODE_ERROR;
}
