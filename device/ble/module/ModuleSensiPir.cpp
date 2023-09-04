#include "ModuleSensiPir.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleSensiPir::ModuleSensiPir(Device *device, uint32_t addr) : Module(device, addr)
{
	sensi = 0;
	id = BLE_ATTRIBUTE_SENSI_SENSOR;
}

ModuleSensiPir::~ModuleSensiPir()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleSensiPir::InitAttribute(int id, double value)
{
	if (this->id == id)
		sensi = value;
}

void ModuleSensiPir::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, sensi);
}
#endif

int ModuleSensiPir::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			sensi = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}

int ModuleSensiPir::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0xe3 && data[1] == 0x11 && data[2] == 0x02 && data[3] == 0x45 && data[4] == 0x05)
	{
		sensi = data[5];
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleSensiPir::CheckData(Json::Value &dataValue, bool &rs)
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
				rs = Util::CompareNumber(op, this->sensi, value1, value2);
				return true;
			}
		}
	}
#endif
	return false;
}

void ModuleSensiPir::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_SENSI] = sensi;
#else
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = sensi;
	jsonValue.append(dataValue);
#endif
}

int ModuleSensiPir::Do(Json::Value &dataValue)
{
	LOGV("ModuleSensiPir Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_SENSI) && dataValue[KEY_ATTRIBUTE_SENSI].isInt())
	{
		int sensi = dataValue[KEY_ATTRIBUTE_SENSI].asInt();
		if (bleProtocol->SetSensiPirLightSensor(addr, sensi) == CODE_OK)
		{
			this->sensi = sensi;
			return CODE_OK;
		}
	}
#else
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			if (bleProtocol)
			{
				bleProtocol->SetSensiPirLightSensor(addr, value);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}
