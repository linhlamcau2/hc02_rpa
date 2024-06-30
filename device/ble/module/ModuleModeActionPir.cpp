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
}

ModuleModeActionPir::~ModuleModeActionPir()
{
}

int ModuleModeActionPir::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_ACMODE) && dataValue[KEY_ATTRIBUTE_ACMODE].isInt())
    {
        mode = dataValue[KEY_ATTRIBUTE_ACMODE].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleModeActionPir::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0xe3 && data[1] == 0x11 && data[2] == 0x02 && data[3] == 0x45 && data[4] == 0x04)
	{
		mode = data[5];
		BuildTelemetryValue(jsonValue);
		CheckTrigger(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleModeActionPir::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_ACMODE) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_ACMODE].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_ACMODE].asInt();
			rs = Util::CompareNumber(op, this->mode, value);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_ACMODE].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_ACMODE];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int value1 = listValue[0].asInt();
				int value2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->mode, value1, value2);
				return true;
			}
		}
	}
	return false;
}

void ModuleModeActionPir::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_ACMODE] = mode;
}

int ModuleModeActionPir::Do(Json::Value &dataValue)
{
	LOGV("ModuleModeActionPir Do data: %s", dataValue.toString().c_str());
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
	return CODE_ERROR;
}
