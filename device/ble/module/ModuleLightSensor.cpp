#include "ModuleLightSensor.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleLightSensor::ModuleLightSensor(Device *device, uint32_t addr) : Module(device, addr)
{
	lux = 0;
	id = BLE_ATTRIBUTE_LUX;
}

ModuleLightSensor::~ModuleLightSensor()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleLightSensor::InitAttribute(int id, double value)
{
	if (this->id == id)
		lux = value;
}

void ModuleLightSensor::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, lux);
}
#endif

int ModuleLightSensor::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
			{
				lux = dataValue["VALUE"].asInt();
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}

int ModuleLightSensor::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52)
	{
		if (data[1] == 0x04 && data[2] == 0x00)
		{
			typedef struct __attribute__((packed))
			{
				uint16_t lux;
				uint16_t scene;
			} data_message_t;
			data_message_t *data_message = (data_message_t *)&data[3];
			lux = (data_message->lux);
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return false;
		}
	}
	return true;
}

bool ModuleLightSensor::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
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
				rs = Util::CompareNumber(op, this->lux, value1, value2);
				return true;
			}
		}
	}
#endif
	return false;
}

// TODO: can nhac di chuyen den Module.cpp
void ModuleLightSensor::CheckTrigger()
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

void ModuleLightSensor::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_LUX] = lux;
#else
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = lux;
	jsonValue.append(dataValue);
#endif
}
