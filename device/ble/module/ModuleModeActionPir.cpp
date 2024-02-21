#include "ModuleModeActionPir.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleModeActionPir::ModuleModeActionPir(Device *device, uint32_t addr) : Module(device, addr)
{
	mode = 0;
	id = BLE_ATTRIBUTE_ACMODE;
}

ModuleModeActionPir::~ModuleModeActionPir()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleModeActionPir::InitAttribute(int id, double value)
{
	if (this->id == id)
		mode = value;
}

void ModuleModeActionPir::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, mode);
}
#endif

int ModuleModeActionPir::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isArray())
	{
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			if (dataValue[i].isObject() && dataValue[i].isMember("ID") && dataValue[i]["ID"].isInt())
			{
				int id = dataValue[i]["ID"].asInt();
				if (this->id == id && dataValue[i].isMember("VALUE") && dataValue[i]["VALUE"].isInt())
				{
					mode = dataValue[i]["VALUE"].asInt();
					BuildTelemetryValue(jsonValue);
					return CODE_OK;
				}
			}
		}
	}
	else if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			mode = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}

int ModuleModeActionPir::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0xe3 && data[1] == 0x11 && data[2] == 0x02 && data[3] == 0x45 && data[4] == 0x04)
	{
		mode = data[5];
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleModeActionPir::CheckData(Json::Value &dataValue, bool &rs)
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
				rs = Util::CompareNumber(op, this->mode, value1, value2);
				return true;
			}
		}
	}
#endif
	return false;
}

void ModuleModeActionPir::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_ACMODE] = mode;
#else
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = mode;
	jsonValue.append(dataValue);
#endif
}

int ModuleModeActionPir::Do(Json::Value &dataValue)
{
	LOGV("ModuleModeActionPir Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (bleProtocol && dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_ACMODE) && dataValue[KEY_ATTRIBUTE_ACMODE].isInt())
	{
		int mode = dataValue[KEY_ATTRIBUTE_ACMODE].asInt();
		if (bleProtocol->SetModeActionPirLightSensor(addr, mode) == CODE_OK)
		{
			this->mode = mode;
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
				bleProtocol->SetModeActionPirLightSensor(addr, value);
			}
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}
