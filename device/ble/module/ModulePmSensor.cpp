#include "ModulePmSensor.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModulePmSensor::ModulePmSensor(Device *device, uint32_t addr) : Module(device, addr)
{
	pm25 = 0;
	pm10 = 0;
	pm1_0 = 0;
	idPm25 = BLE_ATTRIBUTE_PM2_5;
	idPm10 = BLE_ATTRIBUTE_PM10;
	idPm1_0 = BLE_ATTRIBUTE_PM1_0;
}

ModulePmSensor::~ModulePmSensor()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModulePmSensor::InitAttribute(int id, double value)
{
	if (this->id == idPm25)
		pm25 = value;
	else if (this->id == idPm10)
		pm10 = value;
	else if (this->id == idPm1_0)
		pm1_0 = value;
}

void ModulePmSensor::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idPm25, pm25);
	database->DeviceAttributeAddOrReplace(device, idPm10, pm10);
	database->DeviceAttributeAddOrReplace(device, idPm1_0, pm1_0);
}
#endif

int ModulePmSensor::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idPm25 == id || this->idPm10 == id || this->idPm1_0 == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
			{
				if (this->idPm25 == id)
					pm25 = dataValue["VALUE"].asInt();
				else if (this->idPm10 == id)
					pm10 = dataValue["VALUE"].asInt();
				else if (this->idPm1_0 == id)
					pm1_0 = dataValue["VALUE"].asInt();
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}

int ModulePmSensor::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x07 && data[2] == 0x02)
	{
		typedef struct __attribute__((packed))
		{
			uint16_t pm25;
			uint16_t pm10;
			uint16_t pm1_0;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)&data[3];
		pm25 = bswap_16(data_message->pm25);
		pm10 = bswap_16(data_message->pm10);
		pm1_0 = bswap_16(data_message->pm1_0);
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModulePmSensor::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idPm25 == id || this->idPm10 == id || this->idPm1_0 == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
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

					if (this->idPm25 == id)
						rs = Util::CompareNumber(this->pm25, value1, value2, op);
					else if (this->idPm10 == id)
						rs = Util::CompareNumber(this->pm10, value1, value2, op);
					else if (this->idPm1_0 == id)
						rs = Util::CompareNumber(this->pm1_0, value1, value2, op);
					return true;
				}
			}
		}
	}
#endif
	return false;
}

void ModulePmSensor::CheckTrigger()
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

void ModulePmSensor::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_PM2_5] = pm25;
	jsonValue[KEY_ATTRIBUTE_PM10] = pm10;
	jsonValue[KEY_ATTRIBUTE_PM1_0] = pm1_0;
#else
	Json::Value dataValue;
	dataValue["ID"] = idPm25;
	dataValue["VALUE"] = pm25;
	jsonValue.append(dataValue);
	dataValue["ID"] = idPm10;
	dataValue["VALUE"] = pm10;
	jsonValue.append(dataValue);
	dataValue["ID"] = idPm1_0;
	dataValue["VALUE"] = pm1_0;
	jsonValue.append(dataValue);
#endif
}
